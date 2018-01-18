/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "digitizer.hpp"
#include "AcqirisImport.h"   // Common Import for all Agilent Acqiris product families
#include "AcqirisD1Import.h" // Import for Agilent Acqiris Digitizers
#include "simulator.hpp"
#include "sampleprocessor.hpp"
#include <adportable/string.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportable/serializer.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adacquire/signalobserver.hpp>
#include <libdgpio/pio.hpp>
#include <workaround/boost/asio.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/error_code.hpp>
#include <boost/type_traits.hpp>
#include <boost/variant.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <mutex>
#include <thread>
#include <algorithm>
#include <chrono>
#include <atomic>

namespace ap240 {

    namespace detail {
#if _MSC_VER
        const ViInt32 nbrSegments = 1;
#else
        constexpr ViInt32 nbrSegments = 1;
#endif
        enum { pin_A = 1, pin_B = 2, pin_C = 3, pin_TR = 9 };

        class task {
            task();
            ~task();
        public:
            static task * instance();
            
            inline boost::asio::io_service& io_service() { return io_service_; }
            
            void terminate();
            bool initialize();

            bool prepare_for_run( const acqrscontrols::ap240::method& );
            bool run();
            bool stop();
            bool trigger_inject_out();

            void connect( digitizer::command_reply_type f );
            void disconnect( digitizer::command_reply_type f );
            void connect( digitizer::waveform_reply_type f );
            void disconnect( digitizer::waveform_reply_type f );
            void setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw >& ptr );

            inline const acqrscontrols::ap240::method& method() const { return method_; }
            inline const acqrscontrols::ap240::identify& ident() const { return *ident_; }

            inline double timestamp() const {
                return 
                    std::chrono::duration<double>( std::chrono::steady_clock::now() - uptime_ ).count();
            }
            
            static std::string error_msg( int status, const char * ident, ViSession instId = VI_NULL ) {
                std::ostringstream o;
                std::array< ViChar, 1024 > msg;
                std::fill( msg.begin(), msg.end(), 0 );
                
                if ( AcqrsD1_errorMessageEx( instId, status, msg.data(), ViInt32( msg.size() ) ) == VI_SUCCESS ) {
                    o << msg.data() << " 0x" << std::hex << status << " at " << ident;
                } else {
                    o << "ERROR: code =0x" << std::hex << status << " at " << ident;
                }
                return o.str();
            }

            static bool checkError( ViSession instId, ViStatus st, const char * text
                                    , const char * __file__, int __line__, const char * __function__ ) {

                if ( st != VI_SUCCESS )
                    ADWARN() << error_msg( st, text, instId );

                return false;
            }

            static void print( std::ostream& o, const acqrscontrols::ap240::method& m, const char * heading ) {
                o << "-------- " << heading << "---------->" << std::endl;
                o << boost::format( "\ttrig:\tClass: %x,\tPattern: %x,\tCoupling: %x,\tSlope: %x,\tLevel %g, %g" )
                    % m.trig_.trigClass % m.trig_.trigPattern % m.trig_.trigCoupling % m.trig_.trigSlope
                    % m.trig_.trigLevel1 % m.trig_.trigLevel2 << std::endl;
                o << boost::format( "\thoriz:\tsampInterval: %g,\tdelay: %g,\twidth: %g,\tmode: %x,\tflags: %x" )
                    % m.hor_.sampInterval % m.hor_.delayTime % m.hor_.width() % m.hor_.mode % m.hor_.flags << std::endl;
                for ( auto& ch: { m.ext_, m.ch1_, m.ch2_ } ) {
                    o << boost::format( "\tvert: fullScale: %g,\toffset: %g,\tcoupling: %x\t bandwidth: %x" )
                        % ch.fullScale % ch.offset % ch.coupling % ch.bandwidth << std::endl;
                }
                o << "<------- " << heading << "------------" << std::endl;    
            }

            inline ViSession inst() const { return inst_; }

            void reply( const std::string& key, const std::string& value ) {
                for ( auto& handler: reply_handlers_ )
                    handler( key, value );
            }

            bool simulated() const {
                return simulated_;
            }

            int temperature() const {
                return temperature_;
            }

        private:
            static task * instance_;
            static std::mutex mutex_;

            std::vector< adportable::asio::thread > threads_;
            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;
            boost::asio::io_service::strand strand_;
            boost::asio::steady_timer timer_;
            
            bool simulated_;
            std::unique_ptr< dgpio::pio > pio_;
            acqrscontrols::ap240::method method_;
            std::atomic_flag acquire_posted_;
            std::atomic<int> initialize_posted_;
            std::atomic< double > ap240_inject_timepoint_;
            
            bool c_injection_requested_;
            bool c_acquisition_status_; // true := acq. is active, 
            
            std::chrono::steady_clock::time_point uptime_;
            std::chrono::steady_clock::time_point tp_inject_;
            uint32_t data_serialnumber_;
            ViSession inst_;
            ViInt32 numInstruments_;
            std::string device_name_;
            std::string model_name_;
            ViInt32 bus_number_;
            ViInt32 slot_number_;
            ViInt32 serial_number_;
            //ViInt32 nbrSamples_;
            //ViInt32 nStartDelay_;
            //ViInt32 nbrWaveforms_;
            ViInt32 temperature_;
            
            std::vector< digitizer::command_reply_type > reply_handlers_;
            std::vector< digitizer::waveform_reply_type > waveform_handlers_;
            std::shared_ptr< acqrscontrols::ap240::identify > ident_;
            std::shared_ptr< adportable::TimeSquaredScanLaw > scanlaw_;

            bool handle_initial_setup();
            bool handle_terminating();
            bool handle_acquire();
            bool handle_prepare_for_run( const acqrscontrols::ap240::method );
            bool handle_protocol( const acqrscontrols::ap240::method );
            bool handle_temperature();
            void handle_timer( const boost::system::error_code& );
            bool acquire();
            bool waitForEndOfAcquisition( int timeout );
            bool readData( acqrscontrols::ap240::waveform&, int channel );
            // void set_time_since_inject( acqrscontrols::ap240::waveform& );

            bool getInstrumentData() {

                ViChar buf[256];
                ViStatus st = Acqrs_getInstrumentData(inst_, buf, &serial_number_, &bus_number_, &slot_number_);
                if ( st != VI_SUCCESS ) {
                    std::cerr << error_msg(st, "Acqiris::getInstrumentData") << std::endl;
                } else {
                    model_name_ = buf;
                    ident_->bus_number_ = bus_number_;
                    ident_->slot_number_ = slot_number_;
                    ident_->serial_number_ = serial_number_;
                    reply( "InstrumentData",
                           ( boost::format( "Model '%1%', S/N:%2%, bus:slot=%3%:%4%" ) 
                             % model_name_ % serial_number_ % bus_number_ % slot_number_ ).str() );
                }
                return st == VI_SUCCESS;
            }
    
        };

        struct device_ap240 {
            static bool initial_setup( task&, acqrscontrols::ap240::method& );
            static bool protocol_setup( task&, acqrscontrols::ap240::method& );
            static bool acquire( task& );
            static bool waitForEndOfAcquisition( task&, int timeout );
            static bool readData( task&, acqrscontrols::ap240::waveform&, const acqrscontrols::ap240::method&, int channel );
        private:
            static bool averager_setup( task&, acqrscontrols::ap240::method& );
            static bool digitizer_setup( task&, acqrscontrols::ap240::method& );

            template<typename T, typename SegmentDescriptor> static ViStatus
            readData( ViSession inst, const acqrscontrols::ap240::method& m, int channel
                      , AqDataDescriptor& dataDesc, SegmentDescriptor& segDesc, acqrscontrols::ap240::waveform& d ) {

                memset(&dataDesc, 0, sizeof(dataDesc));
                memset(&segDesc, 0, sizeof(segDesc));

                AqReadParameters readPar;
                readPar.dataType = sizeof(T) == 1 ? ReadInt8 : sizeof(T) == 2 ? ReadInt16 : ReadInt32;
                readPar.readMode = m.hor_.mode == 0 ? ReadModeStdW : ReadModeAvgW;
                readPar.firstSegment = 0;
                readPar.nbrSegments = 1;
                readPar.firstSampleInSeg = 0;
                readPar.nbrSamplesInSeg = m.hor_.nbrSamples;
                readPar.segmentOffset = 0;
                readPar.dataArraySize = ( (m.hor_.nbrSamples + 32) * sizeof(T) + sizeof(int32_t) ) & ~3; // octets
                readPar.segDescArraySize = sizeof(segDesc);
                readPar.flags = 0;
                readPar.reserved = 0;
                readPar.reserved2 = 0;
                readPar.reserved3 = 0;
                d.d_.resize( readPar.dataArraySize / sizeof(int32_t) );
                d.method_ = m;
                auto rcode = AcqrsD1_readData( inst, channel, &readPar, d.d_.data(), &dataDesc, &segDesc );
                if ( rcode != VI_SUCCESS )
                    task::checkError( inst, rcode, "readData", __FILE__,__LINE__,__FUNCTION__);
                return rcode;
            }
        };

    }
}

using namespace ap240;
using namespace ap240::detail;

task * task::instance_ = 0;
std::mutex task::mutex_;

digitizer::digitizer()
{
}

digitizer::~digitizer()
{
}

bool
digitizer::peripheral_terminate()
{
    task::instance()->terminate();
    return true;
}

bool
digitizer::peripheral_prepare_for_run( const acqrscontrols::ap240::method& m )
{
    return task::instance()->prepare_for_run( m );
}

bool
digitizer::peripheral_prepare_for_run()
{
    return task::instance()->prepare_for_run( task::instance()->method() );
}

bool
digitizer::peripheral_initialize()
{
    return task::instance()->initialize();
}

bool
digitizer::peripheral_run()
{
    return task::instance()->run();
}

bool
digitizer::peripheral_stop()
{
    return task::instance()->stop();
}

bool
digitizer::peripheral_trigger_inject()
{
    return task::instance()->trigger_inject_out();
}

double
digitizer::temperature() const
{
    return task::instance()->temperature();
}

void
digitizer::connect_reply( command_reply_type f )
{
    task::instance()->connect( f );
}

void
digitizer::disconnect_reply( command_reply_type f )
{
    task::instance()->disconnect( f );
}

void
digitizer::connect_waveform( waveform_reply_type f )
{
    task::instance()->connect( f );
}

void
digitizer::disconnect_waveform( waveform_reply_type f )
{
    task::instance()->disconnect( f );
}

void
digitizer::setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw > ptr )
{
    task::instance()->setScanLaw( ptr );
}

//////////////

task::task() : work_( io_service_ )
             , strand_( io_service_ )
             , timer_( io_service_ )
             , simulated_( false )
             , pio_( std::make_unique< dgpio::pio >() )
             , initialize_posted_( 0 )
             , c_injection_requested_( false )
             , c_acquisition_status_( false )
             , uptime_( std::chrono::steady_clock::now() )
             , tp_inject_( uptime_ )
             , data_serialnumber_( 0 )
             , inst_( -1 )
             , numInstruments_( 0 )
             , serial_number_( 0 )
             , temperature_( 0 )
             , ident_( std::make_shared< acqrscontrols::ap240::identify >() )
{
    acquire_posted_.clear();
    pio_->open();
    
    threads_.push_back( adportable::asio::thread( boost::bind( &boost::asio::io_service::run, &io_service_ ) ) );
}

task::~task()
{
}

task *
task::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new task();
    }
    return instance_;
}

bool
task::initialize()
{
    ADDEBUG() << "## ap240 digitizer initializing...";
    initialize_posted_++;
    strand_.post( [&] { handle_initial_setup(); } );
    return true;
}

bool
task::prepare_for_run( const acqrscontrols::ap240::method& m )
{
    if ( initialize_posted_.load() == 0 )
        return false;

    strand_.post( [this,m] { handle_prepare_for_run(m); } );

    if ( ! std::atomic_flag_test_and_set( &acquire_posted_) ) {
        strand_.post( [this] { handle_acquire(); } );
    }
    
    return true;
}


bool
task::run()
{
    return true;
}

bool
task::stop()
{
    acquire_posted_.clear();
    AcqrsD1_stopAcquisition( inst_ );
    return true;
}

bool
task::trigger_inject_out()
{
    c_injection_requested_ = true;
    return true;
}

void
task::terminate()
{
    timer_.cancel();
    io_service_.stop();
    for ( std::thread& t: threads_ )
        t.join();
    threads_.clear();

    ADDEBUG() << __FUNCTION__ << "##### terminated ######";
}

bool
task::handle_initial_setup()
{
    bool success = false;
    bool simulation = false;
    ViStatus status;

    ADDEBUG() << " ## " << __FUNCTION__ << " ##";
    
    if ( getenv("AcqirisDxDir") == 0 ) {
        ADTRACE() << "AcqirisDxDir environment variable not set.";
        reply( "ap240::digitizer::task::handle_initial_setup", "AcqirisDxDir environment variable not set." );
    }

    if ( auto p = getenv( "AcqirisOption" ) ) {
        if ( strcmp( p, "simulate" ) == 0 )
            simulation = true;
    }
    
#ifdef _LINUX
    if ( ! simulation ) {
        struct stat st;
        if ( stat( "/dev/acqrsPCI", &st ) != 0 ) {
            ADTRACE() << "/dev/acqrsPID does not exists";
            reply( "ap240::digitizer::task::handle_initial_setup", "/dev/acqrsPID does ot exists" );
            return false;
        }
    }
#endif
    status = AcqrsD1_multiInstrAutoDefine( "cal=0", &numInstruments_ );
    if ( status )
        ADTRACE() << error_msg( status, "Acqiris::findDevice()" );

    if ( numInstruments_ == 0 && simulation ) {
        if ( Acqrs_setSimulationOptions( "M2M" ) == VI_SUCCESS )
            numInstruments_ = 1;
        if ( Acqrs_InitWithOptions( const_cast<char*>("PCI::DC271")
                                    , VI_FALSE, VI_FALSE, const_cast<char *>("simulate=TRUE"), &inst_ ) == VI_SUCCESS ) {
            success = true;
            simulated_ = true;
        }
    }
    
    if ( numInstruments_ == 0 )
        return false;

    for ( int i = 0; i < numInstruments_; ++i ) {
        device_name_ = ( boost::format( "PCI::INSTR%1%" ) % i ).str();
        inst_ = ( -1 );
        status = Acqrs_init( const_cast<char *>( device_name_.c_str() ), VI_FALSE, VI_FALSE, &inst_ );
        if ( inst_ != ViSession( -1 ) && getInstrumentData() ) {
            // ADTRACE() << "\tfound device on: " << device_name_;
            success = true;
            break;
        } else {
            ADTRACE() << error_msg( status, "Acqiris::findDevice" );
        }
    }
    
    device_ap240::initial_setup( *this, method_ );

    for ( auto& reply: reply_handlers_ )
        reply( "InitialSetup", ( success ? "success" : "failed" ) );

    using namespace std::chrono_literals;

    timer_.expires_from_now( 5s );
    timer_.async_wait( [&]( const boost::system::error_code& ec ){ handle_timer(ec); } );

	return success;
}

bool
task::handle_terminating()
{
	return false;
}

bool
task::handle_prepare_for_run( const acqrscontrols::ap240::method m )
{
    method_ = m;

    ADDEBUG() << __FUNCTION__ << " >>>>>>>> handle_prepare_for_run <<<<<<<";
    
    device_ap240::initial_setup( *this, method_ );

    return true;
}

bool
task::handle_protocol( const acqrscontrols::ap240::method m )
{
    method_ = m;
    device_ap240::protocol_setup( *this, method_ );
    return true;
}

void
task::handle_timer( const boost::system::error_code& ec )
{
    using namespace std::chrono_literals;

    if ( ec != boost::asio::error::operation_aborted ) {

        strand_.post( [&] { handle_temperature(); } );

        boost::system::error_code erc;
        timer_.expires_from_now( 5s, erc );
        if ( !erc )
            timer_.async_wait( [&]( const boost::system::error_code& ec ){ handle_timer( ec ); });

    }
}

bool
task::handle_temperature()
{
    AcqrsD1_getInstrumentInfo( inst_, "Temperature", &temperature_ );

    std::ostringstream o;
    o << temperature_;
    
    for ( auto& reply: reply_handlers_ )
        reply( "Temperature", o.str() );
    return true;
}

bool
task::handle_acquire()
{
    const static auto epoch = std::chrono::system_clock::from_time_t( 0 );

    if ( std::atomic_flag_test_and_set( &acquire_posted_ ) ) {
        strand_.post( [&] { handle_acquire(); } );    // scedule for next acquire
    } else {
        std::atomic_flag_clear( &acquire_posted_ ); // keep it false
    }

    static auto acquire_tp = std::chrono::system_clock::now();

    if ( acquire() ) {

        if ( waitForEndOfAcquisition( 3000 ) ) {

            auto tp = std::chrono::system_clock::now();
            
            std::shared_ptr< acqrscontrols::ap240::waveform > ch1, ch2;
            
            auto serialnumber = data_serialnumber_++;
            int protocolIndex = pio_->protocol_number(); // <- hard wired protocol id

            // TODO:
            // if ( protocolIndex < 0 && simulated_ )
            //     protocolIndex = simulator::instance()->protocol_number();

            if ( protocolIndex >= 0 ) 
                method_.setProtocolIndex( protocolIndex & 0x03, false ); // low 2bit 
            
            uint32_t events = 0;
            if ( method_.channels_ & 0x01 ) {
                ch1 = std::make_shared< acqrscontrols::ap240::waveform >( *ident_, serialnumber, events, 0 );
                readData( *ch1, 1 );
                ch1->timeSinceEpoch_ = std::chrono::duration_cast<std::chrono::nanoseconds>( tp - epoch ).count();
            }
            
            if ( method_.channels_ & 0x02 ) {
                ch2 = std::make_shared< acqrscontrols::ap240::waveform >( *ident_, serialnumber, events, 0 );
                readData( *ch2, 2 );
                ch2->timeSinceEpoch_ = std::chrono::duration_cast<std::chrono::nanoseconds>( tp - epoch ).count();
            }

            if ( c_injection_requested_ && ( method_.channels_ & 03 ) ) {
                c_injection_requested_ = false;
                c_acquisition_status_ = true;
                ap240_inject_timepoint_ = ( method_.channels_ & 01 ) ? ch1->meta_.initialXTimeSeconds : ch2->meta_.initialXTimeSeconds;
                if ( ch1 )
                    ch1->wellKnownEvents_ |= adacquire::SignalObserver::wkEvent_INJECT;
                if ( ch2 )
                    ch2->wellKnownEvents_ |= adacquire::SignalObserver::wkEvent_INJECT;
            }
            if ( ch1 )
                ch1->timeSinceInject_ = ch1->meta_.initialXTimeSeconds - ap240_inject_timepoint_;
            if ( ch2 )
                ch2->timeSinceInject_ = ch2->meta_.initialXTimeSeconds - ap240_inject_timepoint_;
            
            for ( auto& reply: waveform_handlers_ ) {
                acqrscontrols::ap240::method m;
                if ( reply( ch1.get(), ch2.get(), m ) )
                    handle_protocol( m );
            }
            
            if ( simulated_ ) {
#if defined _MSC_VER && defined _DEBUG
                acquire_tp += std::chrono::milliseconds( 200 ); // 5Hz
#else
                acquire_tp += std::chrono::microseconds( 5000 );  // 200Hz
#endif
                std::this_thread::sleep_until( acquire_tp );
            }
            return true;            

        } else {
            reply( "acquire", "timed out" );
        }
    }
    return false;
}

bool
task::acquire()
{
    return device_ap240::acquire( *this );
}

bool
task::waitForEndOfAcquisition( int timeout )
{
    return device_ap240::waitForEndOfAcquisition( *this, timeout );
}

bool
task::readData( acqrscontrols::ap240::waveform& data, int channel )
{
    return device_ap240::readData( *this, data, method_, channel );
}

void
task::connect( digitizer::command_reply_type f )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    reply_handlers_.push_back( f );
}

void
task::disconnect( digitizer::command_reply_type f )
{
}

void
task::connect( digitizer::waveform_reply_type f )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    waveform_handlers_.push_back( f );
}

void
task::disconnect( digitizer::waveform_reply_type f )
{
    std::lock_guard< std::mutex > lock( mutex_ );    
//	auto it = std::remove_if( waveform_handlers_.begin(), waveform_handlers_.end(), [=]( const digitizer::waveform_reply_type& t ){
//            return t == f;
//        });
//    waveform_handlers_.erase( it, waveform_handlers_.end() );
}

void
task::setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw >& ptr )
{
    scanlaw_ = ptr;
}

bool
device_ap240::initial_setup( task& task, acqrscontrols::ap240::method& m )
{
    ViStatus status;
    ViStatus * pStatus = &status;

    auto inst_ = task.inst();

    if ( ( ( m.channels_ & 03 ) == 03 ) && ( m.hor_.sampInterval < 0.51e-9 ) ) { // if 2ch acquisition, 
        m.hor_.sampInterval = 1.0e-9;
    }

    int nAveragerDelay(0);

    if ( m.hor_.mode == 2 ) {

        m.hor_.nbrSamples = m.hor_.nbrSamples + 32 & ~0x1f; // fold of 32, can't be zero
        double delay = m.hor_.delayTime >= 0 ? m.hor_.delayTime : 0; // averager mode can't be negative
        if ( m.hor_.mode == 0 )
            nAveragerDelay = int32_t( delay / m.hor_.sampInterval + 0.5 ) & ~0x1f;      // fold of 32, can be zero
        else 
            nAveragerDelay = int32_t( delay / m.hor_.sampInterval + 0.5 ) + 32 & ~0x1f; // fold of 32, can't be zero
    }

    // trigger setup
    if ( ( status = AcqrsD1_configTrigClass( inst_
                                             , m.trig_.trigClass, m.trig_.trigPattern, 0, 0, 0, 0 ) )
         == ACQIRIS_WARN_SETUP_ADAPTED ) {
        ViInt32 trigClass, trigPattern, a, b; ViReal64 c, d;
        if ( AcqrsD1_getTrigClass( inst_, &trigClass, &trigPattern, &a, &b, &c, &d ) == VI_SUCCESS ) {
            std::cerr << boost::format( "trigClass: %x <- %x, trigPattern: %x <- %x")
                % trigClass % m.trig_.trigClass % trigPattern % m.trig_.trigPattern << std::endl;
        }
    }
    task::checkError( inst_, status, "AcqrsD1_configTrigClass", __FILE__,__LINE__,__FUNCTION__);
    
    ViInt32 trigChannel = m.trig_.trigPattern & 0x80000000 ? (-1) : m.trig_.trigPattern & 0x3;
    
    status = AcqrsD1_configTrigSource( inst_
                                       , trigChannel
                                       , m.trig_.trigCoupling
                                       , m.trig_.trigSlope //m.ext_trigger_slope // pos(0), neg(1)
                                       , m.trig_.trigLevel1 //m.ext_trigger_level // 500 
                                       , m.trig_.trigLevel2 );
    task::checkError( inst_, status, "AcqrsD1_configTrigSource", __FILE__,__LINE__,__FUNCTION__);

    // vertical setup
    const int chlist [] = { -1, 1, 2 };
    int idx = 0;
    for ( auto& v: { m.ext_, m.ch1_, m.ch2_ } ) {
        int channel = chlist[ idx++ ];
        status = AcqrsD1_configVertical( inst_
                                         , channel
                                         , v.fullScale  
                                         , v.offset     
                                         , v.coupling   
                                         , v.bandwidth );
        // std::cerr << "\tch(" << channel << ") offset: " << v.offset << " fs: " << v.fullScale << std::endl;        
        if ( status == ACQIRIS_WARN_SETUP_ADAPTED ) {
            ViReal64 fullScale, offset; ViInt32 coupling, bandwidth;
            if ( AcqrsD1_getVertical( inst_, channel, &fullScale, &offset, &coupling, &bandwidth ) == VI_SUCCESS ) {
                std::cerr << boost::format( "\tfullscale,offset): (%g,%g) <- (%g,%g)" )
                    % fullScale % offset % v.fullScale % v.offset << std::endl;
            }
        } else
            task::checkError( inst_, status, "configVertical",__FILE__,__LINE__,__FUNCTION__);
    }

    // channels configuration
    if ( m.channels_ == 03 ) { // 2ch simultaneous acquisition

        status = AcqrsD1_configChannelCombination( inst_, 1, m.channels_ ); // all channels use 1 converter each
        task::checkError( inst_, status, "AcqrsD1_configChannelCombination",__FILE__,__LINE__,__FUNCTION__);

    } else {

        status = AcqrsD1_configChannelCombination( inst_, 2, m.channels_ ); // half of the channels use 2 converters each
        task::checkError( inst_, status, "AcqrsD1_configChannelCombination",__FILE__,__LINE__,__FUNCTION__);

    }
    if ( status == ACQIRIS_WARN_SETUP_ADAPTED ) {
        ViInt32 nbrConvertersPerChannel, usedChannels;
        if ( AcqrsD1_getChannelCombination( inst_, &nbrConvertersPerChannel, &usedChannels ) == VI_SUCCESS ) {
            std::cerr << boost::format("ChannelCombination( %x, %x ) <- (%x)" )
                % nbrConvertersPerChannel % &usedChannels % m.channels_ << std::endl;
        }
    }

    if ( m.channels_ & 01 ) {
        status = AcqrsD1_configMultiInput( inst_, 1, 0 ); // channel 1 --> A
        task::checkError( inst_, status, "AcqrsD1_configMultiInput",__FILE__,__LINE__,__FUNCTION__);
    }
    if ( m.channels_ & 02 ) {
        status = AcqrsD1_configMultiInput( inst_, 2, 1 ); // channel 2 --> B
        task::checkError( inst_, status, "AcqrsD1_configMultiInput",__FILE__,__LINE__,__FUNCTION__);
    }
    
    // "IO B" for Acquisition is active
    status = AcqrsD1_configControlIO( inst_, pin_B, 21, 0, 0 );
    task::checkError( inst_, status, "AcqrsD1_configControlIO(B)",__FILE__,__LINE__,__FUNCTION__);
    
    // config trigger out
    status = AcqrsD1_configControlIO( inst_, pin_TR, 1610 / 2, 0, 0 );
    task::checkError( inst_, status, "AcqrsD1_configControlIO(TR)",__FILE__,__LINE__,__FUNCTION__);
    
    if ( m.hor_.mode == 0 )
        return digitizer_setup( task, m );
    else if ( m.hor_.mode == 2 )
        return averager_setup( task, m );

    return false; // unknown mode
}

bool
device_ap240::digitizer_setup( task& task, acqrscontrols::ap240::method& m )
{
    assert( m.hor_.mode == 0 );
    
    auto inst = task.inst();
    ViStatus status;

    if ( ( status = AcqrsD1_configHorizontal( inst, m.hor_.sampInterval, m.hor_.delayTime ) ) == ACQIRIS_WARN_SETUP_ADAPTED ) {
        ViReal64 sampInterval, delay;
        if ( AcqrsD1_getHorizontal( inst, &sampInterval, &delay ) == VI_SUCCESS ) {
            std::cerr << boost::format( "sampInterval: %e <= %e, delay: %e <= %e\n" )
                % sampInterval % m.hor_.sampInterval
                % delay % m.hor_.delayTime;
        }
    }
    task::checkError( inst, status, "AcqrsD1_configHorizontal", __FILE__,__LINE__,__FUNCTION__);

    if ( (status = AcqrsD1_configMemory( inst, m.hor_.nbrSamples, nbrSegments )) == ACQIRIS_WARN_SETUP_ADAPTED ) {
        ViInt32 nbrSamples, nSegments;
        if ( AcqrsD1_getMemory( inst, &nbrSamples, &nSegments ) == VI_SUCCESS )
            m.hor_.nbrSamples = nbrSamples;
    } else 
        task::checkError( inst, status, "configMemory",__FILE__,__LINE__,__FUNCTION__);
    
    status = AcqrsD1_configMode( inst, 0, 0, 0 ); // 2 := averaging mode, 0 := normal data acq.
    task::checkError( inst, status, "AcqrsD1_configMode",__FILE__,__LINE__,__FUNCTION__);

    return true;
}

bool
device_ap240::averager_setup( task& task, acqrscontrols::ap240::method& m )
{
    // averager mode
    assert( m.hor_.mode == 2 );

    auto inst = task.inst();
    ViStatus status;

    status = AcqrsD1_configMode( inst, 2, 0, 0 ); // 2 := averaging mode
    task::checkError( inst, status, "AcqrsD1_configMode",__FILE__,__LINE__,__FUNCTION__);
        
    ViInt32 int32Arg = ViInt32( m.hor_.nbrSamples );
    status = AcqrsD1_configAvgConfig( inst, 0, "NbrSamples", &int32Arg );
    task::checkError( inst, status, "AcqirisD1_configAvgConfig, NbrSamples",__FILE__,__LINE__,__FUNCTION__);

    int32Arg = ViInt32( ( int( m.hor_.delayTime / m.hor_.sampInterval + 0.5 ) + 32 ) & ~0x1f ); // fold of 32, can't be zero
    status = AcqrsD1_configAvgConfig( inst, 0, "StartDelay", &int32Arg );
    task::checkError( inst, status, "AcqrsD1_configAvgConfig StartDelay ", __FILE__,__LINE__,__FUNCTION__);

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst, 0, "StopDelay", &int32Arg );
    task::checkError( inst, status, "AcqrsD1_configAvgConfig (StopDelay)",__FILE__,__LINE__,__FUNCTION__);

    int32Arg = m.hor_.nbrAvgWaveforms;
    status = AcqrsD1_configAvgConfig( inst, 0, "NbrWaveforms", &int32Arg );
    task::checkError( inst, status, "AcqrsD1_configAvgConfig NbrWaveforms ",__FILE__,__LINE__,__FUNCTION__);
        
    int32Arg = 0;
    for ( auto& cmd: { "TrigAlways", "TrigResync", "ThresholdEnable", "TriggerTimeout" } ) {
        status = AcqrsD1_configAvgConfig( inst, 0, cmd, &int32Arg );
        task::checkError( inst, status, cmd,__FILE__,__LINE__,__FUNCTION__);
    }
        
    //-----------------------
    int32Arg = 1;
    for ( auto& cmd: { "TimestampClock", "MarkerLatchMode" } ) {
        status = AcqrsD1_configAvgConfig( inst, 0, cmd, &int32Arg);
        task::checkError( inst, status, cmd,__FILE__,__LINE__,__FUNCTION__);
    }
        
    do {
        int32Arg = m.ch1_.invertData ? 1 : 0; // invert data for mass spectrum;
        status = AcqrsD1_configAvgConfig( inst, 0, "InvertData", &int32Arg); 
        task::checkError( inst, status, "AcqrsD1_configAvgConfig (InvertData)", __FILE__,__LINE__,__FUNCTION__);
    } while (0);


#if 0
    // config "IO A" -- it seems not working for input level
    status = AcqrsD1_configControlIO( inst, 1, 1, 0, 0 );
    if ( task::checkError( inst, status, "AcqrsD1_configControlIO(A)" ) )
        return false;
#endif
    // config "IO B" for Acquisition is active (21)
    status = AcqrsD1_configControlIO( inst, 2, 21, 0, 0 );
    if ( task::checkError( inst, status, "AcqrsD1_configControlIO(B)", __FILE__,__LINE__,__FUNCTION__) )
        return false;

    // Configure the front panel trigger out (TR.)
    // The appropriate offset is 1,610 mV.
    status = AcqrsD1_configControlIO( inst, 9, 1610 / 2, 0, 0 );
    if ( task::checkError( inst, status, "AcqrsD1_configControlIO (2)", __FILE__,__LINE__,__FUNCTION__) )
        return false;

    // "P2Control" set to average(out) --> disable for debug
    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst, 0, "P1Control", &int32Arg );
    task::checkError( inst, status, "AcqrsD1_configAvgConfig (P1Control)",__FILE__,__LINE__,__FUNCTION__);

    // "P2Control" to disable
    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst, 0, "P2Control", &int32Arg );
    task::checkError( inst, status, "AcqrsD1_configAvgConfig (P2Control)",__FILE__,__LINE__,__FUNCTION__);

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst, 0, "DitherRange", &int32Arg );
    task::checkError( inst, status, "AcqrsD1_configAvgConfig (DitherRange)",__FILE__,__LINE__,__FUNCTION__);
	
    int32Arg = nbrSegments;
    status = AcqrsD1_configAvgConfig( inst, 0, "NbrSegments", &int32Arg );
    task::checkError( inst, status, "AcqrsD1_configAvgConfig (NbrSegments)",__FILE__,__LINE__,__FUNCTION__);

    return true;
}

bool
device_ap240::protocol_setup( task& task, acqrscontrols::ap240::method& m )
{
    return device_ap240::initial_setup( task, m );
}

bool
device_ap240::acquire( task& task )
{
    auto rcode = AcqrsD1_acquire( task.inst() );

    if ( rcode != VI_SUCCESS ) {
        task::checkError( task.inst(), rcode, "device_ap240::acquire",__FILE__,__LINE__,__FUNCTION__);

        if ( rcode == ACQIRIS_ERROR_INSTRUMENT_RUNNING )
            AcqrsD1_stopAcquisition( task.inst() );

        std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
    }
    return rcode == VI_SUCCESS;
}

bool
device_ap240::waitForEndOfAcquisition( task& task, int timeout )
{
    return AcqrsD1_waitForEndOfAcquisition( task.inst(), ViInt32( timeout ) ) == VI_SUCCESS;
    // case VI_SUCCESS: return success;
    // case ACQIRIS_ERROR_ACQ_TIMEOUT: return error_timeout;
    // case ACQIRIS_ERROR_OVERLOAD: return error_overload; //  if a channel/trigger overload was detected.
    // case ACQIRIS_ERROR_IO_READ: return error_io_read;   //  if a link error has been detected (e.g. PCI link lost).
    // case ACQIRIS_ERROR_INSTRUMENT_STOPPED: return error_stopped; // if the acquisition was not started beforehand
}

bool
device_ap240::readData( task& task, acqrscontrols::ap240::waveform& data, const acqrscontrols::ap240::method& m, int channel )
{
    data.method_ = m;

    AqDataDescriptor dataDesc;

    // std::cout << "## device_ap240::readData ##" << std::endl;    
    if ( m.hor_.mode == 0 ) {
        AqSegmentDescriptor segDesc;
        if ( readData<int8_t, AqSegmentDescriptor >( task.inst(), m, channel, dataDesc, segDesc, data ) == VI_SUCCESS ) {

            data.timeSinceEpoch_ = std::chrono::steady_clock::now().time_since_epoch().count();
            
            data.meta_.dataType = sizeof( int8_t );
            data.meta_.indexFirstPoint = dataDesc.indexFirstPoint;
            data.meta_.channel = channel;
            data.meta_.actualAverages = 0;
            data.meta_.actualPoints   = dataDesc.returnedSamplesPerSeg; //data.d_.size();
            data.meta_.flags = 0;         // segDesc.flags; // markers not in digitizer
            data.meta_.initialXOffset = data.method_.hor_.delayTime;
            if ( segDesc.timeStampHi == 0 && segDesc.timeStampLo == 0 ) { // digizer mode returns those values 0
                data.meta_.initialXTimeSeconds = task::instance()->timestamp(); // computer's uptime
            } else {
                data.meta_.initialXTimeSeconds = double( uint64_t(segDesc.timeStampHi) << 32 | segDesc.timeStampLo ) / std::pico::den; // ps -> s
            }

            ADDEBUG() << "--------------> " << data.meta_.initialXTimeSeconds;
            
            data.meta_.scaleFactor = dataDesc.vGain;     // V = vGain * data - vOffset
            data.meta_.scaleOffset = dataDesc.vOffset;
            data.meta_.xIncrement = dataDesc.sampTime;
            data.meta_.horPos = segDesc.horPos;
        }
    } else {
        AqSegmentDescriptorAvg segDesc;
        ViStatus rcode(0);
        if ( ( rcode = readData<int32_t, AqSegmentDescriptorAvg>( task.inst(), m, channel, dataDesc, segDesc, data ) ) == VI_SUCCESS ) {

            data.timeSinceEpoch_ = std::chrono::steady_clock::now().time_since_epoch().count();
            
            data.meta_.dataType = sizeof( int32_t );
            data.meta_.indexFirstPoint = dataDesc.indexFirstPoint;
            data.meta_.channel = channel;            
            data.meta_.actualAverages = segDesc.actualTriggersInSeg;  // number of triggers for average
            data.meta_.actualPoints = dataDesc.returnedSamplesPerSeg; // data.d_.size();
            data.meta_.flags = segDesc.flags; // markers
            data.meta_.initialXOffset = dataDesc.sampTime * data.method_.hor_.delayTime;
            uint64_t tstamp = uint64_t(segDesc.timeStampHi) << 32 | segDesc.timeStampLo;
            data.meta_.initialXTimeSeconds = double( tstamp ) / std::pico::den; // ps -> s
            data.meta_.scaleFactor = dataDesc.vGain;
            data.meta_.scaleOffset = dataDesc.vOffset;
            data.meta_.xIncrement = dataDesc.sampTime;
            data.meta_.horPos = segDesc.horPos;
        } else {
            if ( rcode == ACQIRIS_ERROR_READMODE && task::instance()->simulated() ) {

                data.timeSinceEpoch_ = std::chrono::steady_clock::now().time_since_epoch().count();
                
                data.meta_.dataType = sizeof( int32_t );
                data.meta_.indexFirstPoint = 0;
                data.meta_.channel = channel;
                data.meta_.actualAverages = m.hor_.nbrAvgWaveforms;
                data.meta_.actualPoints = m.hor_.nbrSamples;
                data.meta_.flags = 0;
                data.meta_.initialXOffset = m.hor_.delayTime;
                data.meta_.initialXTimeSeconds = task::instance()->timestamp();
                data.meta_.scaleFactor = 1.0;
                data.meta_.scaleOffset = 0.0;
                data.meta_.xIncrement = m.hor_.sampInterval;
                data.meta_.horPos = 0.0;
            }
        }
        ADDEBUG() << "--------------> " << data.meta_.initialXTimeSeconds;        
    }
    return true;
}

