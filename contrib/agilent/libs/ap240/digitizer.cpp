/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <workaround/boost/asio.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
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

            bool prepare_for_run( const ap240controls::method& );
            bool run();
            bool stop();
            bool trigger_inject_out();

            void connect( digitizer::command_reply_type f );
            void disconnect( digitizer::command_reply_type f );
            void connect( digitizer::waveform_reply_type f );
            void disconnect( digitizer::waveform_reply_type f );
            void setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw >& ptr );

            inline const ap240controls::method& method() const { return method_; }
            inline const ap240controls::identify& ident() const { return *ident_; }

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

            static bool checkError( ViSession instId, ViStatus st, const char * text, ViInt32 arg = 0 ) {
                if ( st == VI_SUCCESS )
                    return false;
                std::cerr << error_msg( st, text, instId );
                if ( arg )
                    std::cerr << " #" << arg;
                std::cerr << std::endl;
                return false;
            }

            static void print( std::ostream& o, const ap240controls::method& m, const char * heading ) {
                o << "-------- " << heading << "---------->" << std::endl;
                o << boost::format( "\ttrig:\tClass: %x,\tPattern: %x,\tCoupling: %x,\tSlope: %x,\tLevel %g, %g" )
                    % m.trig_.trigClass % m.trig_.trigPattern % m.trig_.trigCoupling % m.trig_.trigSlope
                    % m.trig_.trigLevel1 % m.trig_.trigLevel2 << std::endl;
                o << boost::format( "\thoriz:\tsampInterval: %g,\tdelay: %g,\twidth: %g,\tmode: %x,\tflags: %x" )
                    % m.hor_.sampInterval % m.hor_.delay % m.hor_.width % m.hor_.mode % m.hor_.flags << std::endl;
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

        private:
            static task * instance_;
            static std::mutex mutex_;

            std::vector< adportable::asio::thread > threads_;
            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;
            boost::asio::io_service::strand strand_;
            bool simulated_;
            ap240controls::method method_;
            std::atomic<int> acquire_post_count_;
            std::chrono::steady_clock::time_point uptime_;
            uint64_t inject_timepoint_;
            uint32_t data_serialnumber_;
            ViSession inst_;
            ViInt32 numInstruments_;
            std::string device_name_;
            std::string model_name_;
            ViInt32 bus_number_;
            ViInt32 slot_number_;
            ViInt32 serial_number_;
            ViInt32 nbrSamples_;
            ViInt32 nStartDelay_;
            ViInt32 nbrWaveforms_;

            std::vector< digitizer::command_reply_type > reply_handlers_;
            std::vector< digitizer::waveform_reply_type > waveform_handlers_;
            std::shared_ptr< ap240controls::identify > ident_;
            std::shared_ptr< adportable::TimeSquaredScanLaw > scanlaw_;

            bool handle_initial_setup();
            bool handle_terminating();
            bool handle_acquire();
            bool handle_prepare_for_run( const ap240controls::method );
            bool handle_protocol( const ap240controls::method );            
            bool acquire();
            bool waitForEndOfAcquisition( int timeout );
            bool readData( ap240controls::waveform&, int channel );

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
            static bool initial_setup( task&, ap240controls::method& );
            static bool protocol_setup( task&, ap240controls::method& );
            static bool acquire( task& );
            static bool waitForEndOfAcquisition( task&, int timeout );
            static bool readData( task&, ap240controls::waveform&, const ap240controls::method&, int channel );
        private:
            static bool averager_setup( task&, ap240controls::method& );
            static bool digitizer_setup( task&, ap240controls::method& );

            template<typename T, typename SegmentDescriptor> static bool
            readData( ViSession inst, const ap240controls::method& m, int channel
                      , AqDataDescriptor& dataDesc, SegmentDescriptor& segDesc, ap240controls::waveform& d ) {

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
                ViStatus rcode;
                if ( ( rcode = AcqrsD1_readData( inst, channel, &readPar, d.d_.data(), &dataDesc, &segDesc ) ) == VI_SUCCESS ) {
                    d.method_ = m;                    
                    return true;
                }
                task::checkError( inst, rcode, "readData", __LINE__ );
                return false;
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
digitizer::peripheral_prepare_for_run( const ap240controls::method& m )
{
    if ( task::instance()->inst() != ViSession( -1 ) )
        return task::instance()->prepare_for_run( m );
    return false;
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
             , simulated_( false )
             , data_serialnumber_( 0 )
             , serial_number_( 0 )
             , acquire_post_count_( 0 )
             , inst_( -1 )
             , numInstruments_( 0 )
             , uptime_( std::chrono::steady_clock::now() )
             , ident_( std::make_shared< ap240controls::identify >() )
{
    threads_.push_back( adportable::asio::thread( boost::bind( &boost::asio::io_service::run, &io_service_ ) ) );
#if defined WIN32
    for ( auto& t: threads_ )
        SetThreadPriority( t.native_handle(), THREAD_PRIORITY_BELOW_NORMAL );
#endif
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
    ADTRACE() << "ap240 digitizer initializing...";
    io_service_.post( strand_.wrap( [&] { handle_initial_setup(); } ) );
    return true;
}

bool
task::prepare_for_run( const ap240controls::method& m )
{
    if ( inst_ == ViSession( -1 ) )
        return false;

    if ( method_.hor_.mode == 0 && acquire_post_count_ ) // digitizer mode
        stop();
    
    io_service_.post( strand_.wrap( [this,m] { handle_prepare_for_run(m); } ) );
    
    if ( acquire_post_count_ == 0 ) {
        // here is the race condition, which if acquie_post_count_ going to zero after it has been tested
        // no 'handle_acquire' will be posted.  Although, it can be recovered by user operation
        acquire_post_count_++;
        io_service_.post( strand_.wrap( [this] { handle_acquire(); } ) );
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
    AcqrsD1_stopAcquisition( inst_ );    
    return true;
}

bool
task::trigger_inject_out()
{
    return true;
}

void
task::terminate()
{
    io_service_.stop();
    for ( std::thread& t: threads_ )
        t.join();
    threads_.clear();
}

bool
task::handle_initial_setup()
{
    bool success = false;
    bool simulation = false;
    ViStatus status;

    if ( getenv("AcqirisDxDir") == 0 ) {
        ADTRACE() << "AcqirisDxDir environment variable not set.";
        reply( "ap240::digitizer::task::handle_initial_setup", "AcqirisDxDir environment variable not set." );
    }

    if ( auto p = getenv( "AcqirisOption" ) ) {
        if ( strcmp( p, "simulate" ) == 0 )
            simulation = true;
    }
    
#ifdef _LINUX
    struct stat st;
    if ( stat( "/dev/acqrsPCI", &st ) != 0 ) {
        ADTRACE() << "/dev/acqrsPID does not exists";
        reply( "ap240::digitizer::task::handle_initial_setup", "/dev/acqrsPID does ot exists" );
        return false;
    }
#endif
    status = AcqrsD1_multiInstrAutoDefine( "cal=0", &numInstruments_ );
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
            ADTRACE() << "\tfound device on: " << device_name_;
            success = true;
            break;
        } else {
            ADTRACE() << error_msg( status, "Acqiris::findDevice" );
        }
    }
    
    device_ap240::initial_setup( *this, method_ );

    for ( auto& reply: reply_handlers_ )
        reply( "InitialSetup", ( success ? "success" : "failed" ) );

	return success;
}

bool
task::handle_terminating()
{
	return false;
}

bool
task::handle_prepare_for_run( const ap240controls::method m )
{
    method_ = m;

    device_ap240::initial_setup( *this, method_ );

    return true;
}

bool
task::handle_protocol( const ap240controls::method m )
{
    method_ = m;
    device_ap240::protocol_setup( *this, method_ );
    return true;
}

bool
task::handle_acquire()
{
    const static auto epoch = std::chrono::system_clock::from_time_t( 0 );

    --acquire_post_count_;

    if ( acquire() ) {
        int retry = 3;
        do {
            if ( waitForEndOfAcquisition( 3000 ) ) {
                auto tp = std::chrono::system_clock::now();

                std::shared_ptr< ap240controls::waveform > ch1, ch2;

                auto serialnumber = data_serialnumber_++;
                
                uint32_t events = 0;
                if ( method_.channels_ & 0x01 ) {
                    ch1 = std::make_shared< ap240controls::waveform >( *ident_, events, serialnumber );
                    ch1->serialnumber_ = serialnumber;
                    readData( *ch1, 1 );

                    // On windows 8.1 (x86_64), time_since_epoch() return exactly same value as following
                    // though c++ standard does not specify epoch.
                    ch1->timeSinceEpoch_ = std::chrono::duration_cast<std::chrono::nanoseconds>( tp - epoch ).count();
                    // uint64_t epoch_time = std::chrono::duration_cast<std::chrono::nanoseconds>( tp.time_since_epoch() ).count();
                }

                if ( method_.channels_ & 0x02 ) {
                    ch2 = std::make_shared< ap240controls::waveform >( *ident_, events, serialnumber );
                    ch2->serialnumber_ = serialnumber;
                    readData( *ch2, 2 );
                    ch2->timeSinceEpoch_ = std::chrono::duration_cast<std::chrono::nanoseconds>( tp - epoch ).count();
                }

                for ( auto& reply: waveform_handlers_ ) {
                    ap240controls::method m;
                    if ( reply( ch1.get(), ch2.get(), m ) )
                        handle_protocol( m );
                }

                if ( simulated_ ) {
                    std::this_thread::sleep_until( tp + std::chrono::microseconds( 2000 ) );
                }
                
                ++acquire_post_count_;
                io_service_.post( strand_.wrap( [=] { handle_acquire(); } ) );    // scedule for next acquire

                return true;            

            } else {
                reply( "acquire", "timed out" );
            }
            
            ++acquire_post_count_;
            io_service_.post( strand_.wrap( [=] { handle_acquire(); } ) );    // scedule for next acquire    

        } while ( retry-- );
        
        reply( "acquire", "timed out, stop acquisition" );
        AcqrsD1_stopAcquisition( inst_ );
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
task::readData( ap240controls::waveform& data, int channel )
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
device_ap240::initial_setup( task& task, ap240controls::method& m )
{
    ViStatus status;
    ViStatus * pStatus = &status;

    auto inst_ = task.inst();

    if ( ( ( m.channels_ & 03 ) == 03 ) && ( m.hor_.sampInterval < 0.51e-9 ) ) { // if 2ch acquisition, 
        m.hor_.sampInterval = 1.0e-9;
    }

    if ( m.hor_.mode == 0 ) {
        m.hor_.nbrSamples = uint32_t( m.hor_.width / m.hor_.sampInterval + 0.5 );
        m.hor_.nStartDelay = uint32_t( m.hor_.delay / m.hor_.sampInterval ); // nominal -- not in use
    } else {
        m.hor_.nbrSamples = uint32_t( m.hor_.width / m.hor_.sampInterval + 0.5 ) + 32 & ~0x1f; // fold of 32, can't be zero
        if ( m.hor_.mode == 0 )
            m.hor_.nStartDelay = uint32_t( m.hor_.delay / m.hor_.sampInterval + 0.5 ) & ~0x1f; // fold of 32, can be zero
        else 
            m.hor_.nStartDelay = uint32_t( m.hor_.delay / m.hor_.sampInterval + 0.5 ) + 32 & ~0x1f; // fold of 32, cannt be zero for average mode
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
    task::checkError( inst_, status, "AcqrsD1_configTrigClass", __LINE__  );
    
    ViInt32 trigChannel = m.trig_.trigPattern & 0x80000000 ? (-1) : m.trig_.trigPattern & 0x3;
    
    status = AcqrsD1_configTrigSource( inst_
                                       , trigChannel
                                       , m.trig_.trigCoupling
                                       , m.trig_.trigSlope //m.ext_trigger_slope // pos(0), neg(1)
                                       , m.trig_.trigLevel1 //m.ext_trigger_level // 500 
                                       , m.trig_.trigLevel2 );
    task::checkError( inst_, status, "AcqrsD1_configTrigSource", __LINE__  );

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
            task::checkError( inst_, status, "configVertical", __LINE__ );
    }

    // channels configuration
    if ( m.channels_ == 03 ) { // 2ch simultaneous acquisition

        status = AcqrsD1_configChannelCombination( inst_, 1, m.channels_ ); // all channels use 1 converter each
        task::checkError( inst_, status, "AcqrsD1_configChannelCombination", __LINE__  );

    } else {

        status = AcqrsD1_configChannelCombination( inst_, 2, m.channels_ ); // half of the channels use 2 converters each
        task::checkError( inst_, status, "AcqrsD1_configChannelCombination", __LINE__  );

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
        task::checkError( inst_, status, "AcqrsD1_configMultiInput", __LINE__  );
    }
    if ( m.channels_ & 02 ) {
        status = AcqrsD1_configMultiInput( inst_, 2, 1 ); // channel 2 --> B
        task::checkError( inst_, status, "AcqrsD1_configMultiInput", __LINE__  );
    }
    
    // "IO B" for Acquisition is active
    status = AcqrsD1_configControlIO( inst_, pin_B, 21, 0, 0 );
    task::checkError( inst_, status, "AcqrsD1_configControlIO(B)", __LINE__  );
    
    // config trigger out
    status = AcqrsD1_configControlIO( inst_, pin_TR, 1610 / 2, 0, 0 );
    task::checkError( inst_, status, "AcqrsD1_configControlIO(TR)", __LINE__  );
    
    if ( m.hor_.mode == 0 )
        return digitizer_setup( task, m );
    else if ( m.hor_.mode == 2 )
        return averager_setup( task, m );

    return false; // unknown mode
}

bool
device_ap240::digitizer_setup( task& task, ap240controls::method& m )
{
    assert( m.hor_.mode == 0 );
    
    auto inst = task.inst();
    ViStatus status;

    if ( ( status = AcqrsD1_configHorizontal( inst, m.hor_.sampInterval, m.hor_.delay ) ) == ACQIRIS_WARN_SETUP_ADAPTED ) {
        ViReal64 sampInterval, delay;
        if ( AcqrsD1_getHorizontal( inst, &sampInterval, &delay ) == VI_SUCCESS ) {
            std::cerr << boost::format( "sampInterval: %e <= %e, delay: %e <= %e\n" )
                % sampInterval % m.hor_.sampInterval
                % delay % m.hor_.delay;
        }
    }
    task::checkError( inst, status, "AcqrsD1_configHorizontal", __LINE__  );

    if ( (status = AcqrsD1_configMemory( inst, m.hor_.nbrSamples, nbrSegments )) == ACQIRIS_WARN_SETUP_ADAPTED ) {
        ViInt32 nbrSamples, nSegments;
        if ( AcqrsD1_getMemory( inst, &nbrSamples, &nSegments ) == VI_SUCCESS )
            m.hor_.nbrSamples = nbrSamples;
    } else 
        task::checkError( inst, status, "configMemory", __LINE__ );
    
    status = AcqrsD1_configMode( inst, 0, 0, 0 ); // 2 := averaging mode, 0 := normal data acq.
    task::checkError( inst, status, "AcqrsD1_configMode", __LINE__  );

    return true;
}

bool
device_ap240::averager_setup( task& task, ap240controls::method& m )
{
    // averager mode
    assert( m.hor_.mode == 2 );

    // std::cout << "###### setup for averager #######" << std::endl;
    
    auto inst = task.inst();
    ViStatus status;

    status = AcqrsD1_configMode( inst, 2, 0, 0 ); // 2 := averaging mode
    task::checkError( inst, status, "AcqrsD1_configMode", __LINE__  );
        
    ViInt32 int32Arg = ViInt32( m.hor_.nbrSamples );
    status = AcqrsD1_configAvgConfig( inst, 0, "NbrSamples", &int32Arg );
    task::checkError( inst, status, "AcqirisD1_configAvgConfig, NbrSamples", __LINE__ );

    int32Arg = ViInt32( m.hor_.nStartDelay );
    status = AcqrsD1_configAvgConfig( inst, 0, "StartDelay", &int32Arg );
    task::checkError( inst, status, "AcqrsD1_configAvgConfig StartDelay ", __LINE__ );

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst, 0, "StopDelay", &int32Arg );
    task::checkError( inst, status, "AcqrsD1_configAvgConfig (StopDelay)", __LINE__  );

    int32Arg = m.hor_.nbrAvgWaveforms;
    status = AcqrsD1_configAvgConfig( inst, 0, "NbrWaveforms", &int32Arg );
    task::checkError( inst, status, "AcqrsD1_configAvgConfig NbrWaveforms ", __LINE__ );
        
    int32Arg = 0;
    for ( auto& cmd: { "TrigAlways", "TrigResync", "ThresholdEnable", "TriggerTimeout" } ) {
        status = AcqrsD1_configAvgConfig( inst, 0, cmd, &int32Arg );
        task::checkError( inst, status, cmd, __LINE__  );
    }
        
    //-----------------------
    int32Arg = 1;
    for ( auto& cmd: { "TimestampClock", "MarkerLatchMode" } ) {
        status = AcqrsD1_configAvgConfig( inst, 0, cmd, &int32Arg);
        task::checkError( inst, status, cmd, __LINE__  );
    }
        
    do {
        int32Arg = m.ch1_.invertData ? 1 : 0; // invert data for mass spectrum;
        status = AcqrsD1_configAvgConfig( inst, 0, "InvertData", &int32Arg); 
        task::checkError( inst, status, "AcqrsD1_configAvgConfig (InvertData)", __LINE__  );
    } while (0);


#if 0
    // config "IO A" -- it seems not working for input level
    status = AcqrsD1_configControlIO( inst, 1, 1, 0, 0 );
    if ( task::checkError( inst, status, "AcqrsD1_configControlIO(A)" ) )
        return false;
#endif
    // config "IO B" for Acquisition is active (21)
    status = AcqrsD1_configControlIO( inst, 2, 21, 0, 0 );
    if ( task::checkError( inst, status, "AcqrsD1_configControlIO(B)", __LINE__  ) )
        return false;

    // Configure the front panel trigger out (TR.)
    // The appropriate offset is 1,610 mV.
    status = AcqrsD1_configControlIO( inst, 9, 1610 / 2, 0, 0 );
    if ( task::checkError( inst, status, "AcqrsD1_configControlIO (2)", __LINE__  ) )
        return false;

    // "P2Control" set to average(out) --> disable for debug
    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst, 0, "P1Control", &int32Arg );
    task::checkError( inst, status, "AcqrsD1_configAvgConfig (P1Control)", __LINE__  );

    // "P2Control" to disable
    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst, 0, "P2Control", &int32Arg );
    task::checkError( inst, status, "AcqrsD1_configAvgConfig (P2Control)", __LINE__  );

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst, 0, "DitherRange", &int32Arg );
    task::checkError( inst, status, "AcqrsD1_configAvgConfig (DitherRange)", __LINE__  );
	
    int32Arg = nbrSegments;
    status = AcqrsD1_configAvgConfig( inst, 0, "NbrSegments", &int32Arg );
    task::checkError( inst, status, "AcqrsD1_configAvgConfig (NbrSegments)", __LINE__  );

    return true;
}

bool
device_ap240::protocol_setup( task& task, ap240controls::method& m )
{
    return device_ap240::initial_setup( task, m );
}

bool
device_ap240::acquire( task& task )
{
    auto rcode = AcqrsD1_acquire( task.inst() );
    if ( rcode != VI_SUCCESS )
        task::checkError( task.inst(), rcode, "device_ap240::acquire", __LINE__ );
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
device_ap240::readData( task& task, ap240controls::waveform& data, const ap240controls::method& m, int channel )
{
    data.method_ = m;

    AqDataDescriptor dataDesc;

    // std::cout << "## device_ap240::readData ##" << std::endl;    
    if ( m.hor_.mode == 0 ) {
        AqSegmentDescriptor segDesc;
        if ( readData<int8_t, AqSegmentDescriptor >( task.inst(), m, channel, dataDesc, segDesc, data ) ) {

            data.timeSinceEpoch_ = std::chrono::steady_clock::now().time_since_epoch().count();

            // std::cout << boost::format( "Time: [%x][%x]" ) % segDesc.timeStampHi % segDesc.timeStampLo << std::endl;
            data.meta_.dataType = sizeof( int8_t );
            data.meta_.indexFirstPoint = dataDesc.indexFirstPoint;
            data.meta_.channel = channel;
            data.meta_.actualAverages = 0;
            data.meta_.actualPoints   = dataDesc.returnedSamplesPerSeg; //data.d_.size();
            data.meta_.flags = 0;         // segDesc.flags; // markers not in digitizer
            data.meta_.initialXOffset = dataDesc.sampTime * data.method_.hor_.nStartDelay;
            double acquire_time = double( uint64_t(segDesc.timeStampHi) << 32 | segDesc.timeStampLo ) * 1.0e-12;  // time since 'acquire' issued
            data.meta_.initialXTimeSeconds = task::instance()->timestamp(); // computer's uptime
            
            data.meta_.scaleFactor = dataDesc.vGain;     // V = vGain * data - vOffset
            data.meta_.scaleOffset = dataDesc.vOffset;
            data.meta_.xIncrement = dataDesc.sampTime;
            data.meta_.horPos = segDesc.horPos;
        }
    } else {
        AqSegmentDescriptorAvg segDesc;
        if ( readData<int32_t, AqSegmentDescriptorAvg>( task.inst(), m, channel, dataDesc, segDesc, data ) ) {

            data.meta_.dataType = sizeof( int32_t );
            data.meta_.indexFirstPoint = dataDesc.indexFirstPoint;

            data.meta_.channel = channel;            
            data.meta_.actualAverages = segDesc.actualTriggersInSeg;  // number of triggers for average
            data.meta_.actualPoints = dataDesc.returnedSamplesPerSeg; //data.d_.size();
            data.meta_.flags = segDesc.flags; // markers
            data.meta_.initialXOffset = dataDesc.sampTime * data.method_.hor_.nStartDelay;
            uint64_t tstamp = uint64_t(segDesc.timeStampHi) << 32 | segDesc.timeStampLo;
            data.meta_.initialXTimeSeconds = double( tstamp )* 1.0e-12;
            data.meta_.scaleFactor = dataDesc.vGain;
            data.meta_.scaleOffset = dataDesc.vOffset;
            data.meta_.xIncrement = dataDesc.sampTime;
            data.meta_.horPos = segDesc.horPos;
        }        
    }
    return true;
}

#if 0
size_t
waveform::size() const
{
    return method_.hor_.nbrSamples;
}

template<> const int8_t *
waveform::begin() const
{
    if ( meta_.dataType != sizeof(int8_t) )
        throw std::bad_cast();
    return reinterpret_cast< const int8_t* >( d_.data() ) + meta_.indexFirstPoint;
}

template<> const int8_t *
waveform::end() const
{
    if ( meta_.dataType != sizeof(int8_t) )
        throw std::bad_cast();    
    return reinterpret_cast< const int8_t* >( d_.data() ) + meta_.indexFirstPoint + method_.hor_.nbrSamples;
}

template<> const int16_t *
waveform::begin() const
{
    if ( meta_.dataType != sizeof(int16_t) )
        throw std::bad_cast();        
    return reinterpret_cast< const int16_t* >( d_.data() ) + meta_.indexFirstPoint;
}

template<> const int16_t *
waveform::end() const
{
    if ( meta_.dataType != sizeof(int16_t) )
        throw std::bad_cast();
    return reinterpret_cast< const int16_t* >( d_.data() ) + meta_.indexFirstPoint + method_.hor_.nbrSamples;
}

template<> const int32_t *
waveform::begin() const
{
    if ( meta_.dataType != sizeof(int32_t) )
        throw std::bad_cast();                
    return reinterpret_cast< const int32_t* >( d_.data() ) + meta_.indexFirstPoint;
}

template<> const int32_t *
waveform::end() const
{
    if ( meta_.dataType != sizeof(int32_t) )
        throw std::bad_cast();                    
    return reinterpret_cast< const int32_t* >( d_.data() ) + meta_.indexFirstPoint + method_.hor_.nbrSamples;
}

std::pair<double, int>
waveform::operator [] ( size_t idx ) const
{
    double time = idx * meta_.xIncrement + meta_.horPos;

    if ( method_.hor_.mode == 0 )
        time += method_.hor_.delay;
    else
        time += method_.hor_.nStartDelay * meta_.xIncrement;
    
    switch( meta_.dataType ) {
    case 1: return std::make_pair( time, *(begin<int8_t>()  + idx) );
    case 2: return std::make_pair( time, *(begin<int16_t>() + idx) );
    case 4: return std::make_pair( time, *(begin<int32_t>() + idx) );
    }
    throw std::exception();
}

double
waveform::toVolts( int d ) const
{
    if ( meta_.actualAverages == 0 )
        return meta_.scaleFactor * d - meta_.scaleOffset;
    else
        return double( meta_.scaleFactor * d ) / meta_.actualAverages - ( meta_.scaleOffset * meta_.actualAverages );
}

double
waveform::toVolts( double d ) const
{
    return d * meta_.scaleFactor /  meta_.actualAverages;
}

namespace ap240 {

    bool method::archive( std::ostream& os, const method& t )
    {
        try {
            portable_binary_oarchive ar( os );
            ar & t;
            return true;
        } catch ( std::exception& ex ) {
            BOOST_THROW_EXCEPTION( ex );
        }
        return false;
    }

    bool method::restore( std::istream& is, method& t )
    {
        try {
            portable_binary_iarchive ar( is );
            ar & t;
            return true;
        } catch ( std::exception& ex ) {
            BOOST_THROW_EXCEPTION( ex );
        }
        return false;
    }

    bool method::xml_archive( std::wostream& os, const method& t )
    {
        try {
            boost::archive::xml_woarchive ar( os );
            ar & boost::serialization::make_nvp( "ap240_method", t );
            return true;
        } catch ( std::exception& ex ) {
            BOOST_THROW_EXCEPTION( ex );
        }
        return false;
    }

    bool method::xml_restore( std::wistream& is, method& t )
    {
        try {
            boost::archive::xml_wiarchive ar( is );
            ar & boost::serialization::make_nvp( "ap240_method", t );
            return true;
        } catch ( std::exception& ex ) {
            BOOST_THROW_EXCEPTION( ex );
        }
        return false;

    }
}
#endif
