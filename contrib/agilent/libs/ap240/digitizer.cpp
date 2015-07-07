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
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <workaround/boost/asio.hpp>
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

        constexpr ViInt32 nbrSegments = 1;

        struct device_ap240 {
            static bool initial_setup( task&, method& );
            static bool protocol_setup( task&, method& );
            static bool acquire( task& );
            static bool waitForEndOfAcquisition( task&, int timeout );
            static bool readData( task&, waveform& );
        private:
            static bool averager_setup( task&, const method& );
            static bool digitizer_setup( task&, const method& );
        };

        class task {
            task();
            ~task();
        public:
            static task * instance();
            
            inline boost::asio::io_service& io_service() { return io_service_; }
            
            void terminate();
            bool initialize();

            bool prepare_for_run( const ap240::method& );
            bool run();
            bool stop();
            bool trigger_inject_out();

            void connect( digitizer::command_reply_type f );
            void disconnect( digitizer::command_reply_type f );
            void connect( digitizer::waveform_reply_type f );
            void disconnect( digitizer::waveform_reply_type f );
            void setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw >& ptr );

            inline const ap240::method& method() const { return method_; }
            inline const identify& ident() const { return *ident_; }
            
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
            ap240::method method_;
            std::atomic<int> acquire_post_count_;
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
            std::shared_ptr< identify > ident_;
            std::shared_ptr< adportable::TimeSquaredScanLaw > scanlaw_;

            bool handle_initial_setup();
            bool handle_terminating();
            bool handle_acquire();
            bool handle_prepare_for_run( const ap240::method );
            bool handle_protocol( const ap240::method );            
            bool acquire();
            bool waitForEndOfAcquisition( int timeout );
            bool readData( waveform& );

            bool getInstrumentData() {

                ViChar buf[256];
                ViStatus st = Acqrs_getInstrumentData(inst_, buf, &serial_number_, &bus_number_, &slot_number_);
                if ( st != VI_SUCCESS ) {
                    std::cerr << error_msg(st, "Acqiris::getInstrumentData") << std::endl;
                } else {
                    model_name_ = buf;
                    reply( "InstrumentData",
                           ( boost::format( "Model '%1%', S/N:%2%, bus:slot=%3%:%4%" ) 
                             % model_name_ % serial_number_ % bus_number_ % slot_number_ ).str() );
                }
                return st == VI_SUCCESS;
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

// bool
// digitizer::peripheral_prepare_for_run( const adcontrols::ControlMethod& m )
// {
//     using adcontrols::controlmethod::MethodItem;

//     std::cout << "digitizer::prepare_for_run" << std::endl;
    
//     adcontrols::ControlMethod cm( m );
//     cm.sort();
//     auto it = std::find_if( cm.begin(), cm.end(), [] ( const MethodItem& mi ){ return mi.modelname() == "ap240"; } );
//     if ( it != cm.end() ) {
//         ap240::method m;
//         if ( adportable::serializer< ap240::method >::deserialize( m, it->data(), it->size() ) ) {
//             return task::instance()->prepare_for_run( m );
//         }
//     }
//     return false;
// }

bool
digitizer::peripheral_prepare_for_run( const ap240::method& m )
{
    return task::instance()->prepare_for_run( m );
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
{
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
    ADTRACE() << "ap240 digitizer initializing...";
    io_service_.post( strand_.wrap( [&] { handle_initial_setup(); } ) );
    return true;
}

bool
task::prepare_for_run( const ap240::method& m )
{
    io_service_.post( strand_.wrap( [&] { handle_prepare_for_run(m); } ) );
    
    if ( acquire_post_count_ == 0 ) {
        acquire_post_count_++;
        io_service_.post( strand_.wrap( [&] { handle_acquire(); } ) );
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

    ViStatus status;

    if ( getenv("AcqirisDxDir") == 0 ) {
        ADTRACE() << "AcqirisDxDir environment variable not set.";
        reply( "ap240::digitizer::task::handle_initial_setup", "AcqirisDxDir environment variable not set." );
    }
    
#ifdef _LINUX
    struct stat st;
    if ( stat( "/dev/acqrsPCI", &st ) != 0 ) {
        ADTRACE() << "/dev/acqrsPID does not exists";
        reply( "ap240::digitizer::task::handle_initial_setup", "/dev/acqrsPID does ot exists" );
        return false;
    }
#endif
    if ( ( status = AcqrsD1_multiInstrAutoDefine( "cal=0", &numInstruments_ ) ) != VI_SUCCESS ) {
        ADTRACE() << error_msg( status, "Acqiris::findDevice()" );
    } else {
        ADTRACE() << boost::format( "find %1% acqiris devices." ) % numInstruments_;
    }
    
    if ( numInstruments_ == 0 )
        return false;
    
    for ( int i = 0; i < numInstruments_; ++i ) {
        device_name_ = ( boost::format( "PCI::INSTR%1%" ) % i ).str();
        inst_ = (-1);
        status = Acqrs_init( const_cast< char *>(device_name_.c_str()), VI_FALSE, VI_FALSE, &inst_);
        if ( inst_ != ViSession(-1) && getInstrumentData() ) {
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
task::handle_prepare_for_run( const ap240::method m )
{
    method_ = m;
    device_ap240::initial_setup( *this, method_ );
    return true;
}

bool
task::handle_protocol( const ap240::method m )
{
    method_ = m;
    device_ap240::protocol_setup( *this, method_ );
    return true;
}

bool
task::handle_acquire()
{
    --acquire_post_count_;
    if ( acquire() ) {
        int retry = 3;
        do {
            if ( waitForEndOfAcquisition( 3000 ) ) {

                auto avgr = std::make_shared< waveform >( ident_ );
                if ( readData( *avgr ) ) {
                    for ( auto& reply: waveform_handlers_ ) {
                        ap240::method m;
                        if ( reply( avgr.get(), m ) )
                            handle_protocol( m );
                    }
                }

                ++acquire_post_count_;
                io_service_.post( strand_.wrap( [=] { handle_acquire(); } ) );    // scedule for next acquire

                return true;            

            } else {
                reply( "acquire", "timed out" );
            }
        } while ( retry-- );
        reply( "acquire", "timed out, stop acquisition" );
        AcqrsD1_stopAcquisition( inst_ );
    }

    ++acquire_post_count_;
    io_service_.post( strand_.wrap( [=] { handle_acquire(); } ) );    // scedule for next acquire    
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
task::readData( waveform& data )
{
    data.serialnumber_ = data_serialnumber_++;
    return device_ap240::readData( *this, data );
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
    std::lock_guard< std::mutex > lock( mutex_ );    
//	auto it = std::remove_if( reply_handlers_.begin(), reply_handlers_.end(), [=]( const digitizer::command_reply_type& t ){
//            return t == f;
//        });
//    reply_handlers_.erase( it, reply_handlers_.end() );
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

// void
// task::error_reply( const _com_error& e, const std::string& method )
// {
//     _bstr_t msg( _bstr_t(L"Error: ") + e.Description() + L": " + e.ErrorMessage() );
//     for ( auto& reply: reply_handlers_ )
//         reply( method, static_cast< const char *>( msg ) );
// }

void
task::setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw >& ptr )
{
    scanlaw_ = ptr;
}

///
identify::identify() : bus_number_(0)
                     , slot_number_(0)
                     , serial_number_(0)
{
}

identify::identify( const identify& t ) : bus_number_(t.bus_number_)
                                        , slot_number_(t.slot_number_)
                                        , serial_number_(t.serial_number_)
{
}

bool
device_ap240::initial_setup( task& task, method& m )
{
    ViStatus status;
    ViStatus * pStatus = &status;

    auto inst_ = task.inst();

    if ( m.hor_.mode == 0 ) {
        m.hor_.nbrSamples = uint32_t( m.hor_.width / m.hor_.sampInterval + 0.5 );
        m.hor_.nStartDelay = uint32_t( m.hor_.delay / m.hor_.sampInterval ); // nominal -- not in use
    } else {
        m.hor_.nbrSamples = uint32_t( m.hor_.width / m.hor_.sampInterval + 0.5 ) + 32 & ~0x1f; // fold of 32, can't be zero
        m.hor_.nStartDelay = uint32_t( m.hor_.delay / m.hor_.sampInterval + 0.5 ) & ~0x1f; // fold of 32, can be zero
    }

    std::cout << " delay: " << m.hor_.nStartDelay << ", width: " << m.hor_.nbrSamples << std::endl;

    // trigger setup
    status = AcqrsD1_configTrigClass( inst_, m.trig_.trigClass, m.trig_.trigPattern, 0, 0, 0, 0 );
    task::checkError( inst_, status, "AcqrsD1_configTrigClass", __LINE__  );
    
    ViInt32 trigChannel = m.trig_.trigPattern & 0x80000000 ? (-1) : m.trig_.trigPattern & 0x3;
    
    status = AcqrsD1_configTrigSource( inst_
                                       , trigChannel
                                       , m.trig_.trigCoupling
                                       , m.trig_.trigSlope //m.ext_trigger_slope // pos(0), neg(1)
                                       , m.trig_.trigLevel1 //m.ext_trigger_level // 500 
                                       , m.trig_.trigLevel2 );
    task::checkError( inst_, status, "AcqrsD1_configTrigSource", __LINE__  );

    typedef std::pair< ViInt32, const method::vertical_method& > channel_method;

    // vertical setup
    for ( auto& ch: { channel_method( -1, m.ext_ ), channel_method( 1, m.ch1_ ), channel_method( 2, m.ch2_ ) } ) {
        
        status = AcqrsD1_configVertical( inst_
                                         , ch.first
                                         , ch.second.fullScale // ..ext_trigger_range // 0.5
                                         , ch.second.offset // m.ext_trigger_offset // 0.0 
                                         , ch.second.coupling   // DC 50ohm
                                         , ch.second.bandwidth ); // ext_trigger_bandwidth /* 700MHz */);
        task::checkError( inst_, status, "configVertical", __LINE__ );
    }

    // channels configuration
    if ( m.channels_ == 03 ) { // 2ch simultaneous acquisition

        if ( m.hor_.sampInterval < 0.51e-9 )
            m.hor_.sampInterval = 1.0e-9;

        status = AcqrsD1_configChannelCombination( inst_, 1, m.channels_ ); // all channels use 1 converter each
        task::checkError( inst_, status, "AcqrsD1_configChannelCombination", __LINE__  );

    } else {

        status = AcqrsD1_configChannelCombination( inst_, 2, m.channels_ ); // half of the channels use 2 converters each
        task::checkError( inst_, status, "AcqrsD1_configChannelCombination", __LINE__  );

    }

    if ( m.channels_ & 01 ) {
        status = AcqrsD1_configMultiInput( inst_, 1, 0 ); // channel 1 --> A
        task::checkError( inst_, status, "AcqrsD1_configMultiInput", __LINE__  );
    }
    if ( m.channels_ & 02 ) {
        status = AcqrsD1_configMultiInput( inst_, 2, 1 ); // channel 2 --> B
        task::checkError( inst_, status, "AcqrsD1_configMultiInput", __LINE__  );
    }
    
    if ( m.hor_.mode == 0 )
        return digitizer_setup( task, m );
    else if ( m.hor_.mode == 2 )
        return averager_setup( task, m );
}

bool
device_ap240::digitizer_setup( task& task, const method& m )
{
    assert( m.hor_.mode == 0 );
    
    auto inst = task.inst();
    
    std::cerr << boost::format("AcqrsD1_configMode( %1%, mode=%2%, flags=%3% )") % inst % m.hor_.mode % m.hor_.flags << std::endl;
    ViStatus status = AcqrsD1_configMode( inst, m.hor_.mode, 0, m.hor_.flags ); // 2 := averaging mode, 0 := normal data acq.
    task::checkError( inst, status, "AcqrsD1_configMode", __LINE__  );

    status = AcqrsD1_configHorizontal( inst, m.hor_.sampInterval, m.hor_.delay );
    task::checkError( inst, status, "AcqrsD1_configHorizontal", __LINE__  );

    std::cerr << boost::format("AcqrsD1_configMemory( %1%, nbrSamples=%2% )") % inst % m.hor_.nbrSamples << std::endl;
    status = AcqrsD1_configMemory( inst, m.hor_.nbrSamples, nbrSegments );
    task::checkError( inst, status, "configMemory", __LINE__ );

    return true;
}

bool
device_ap240::averager_setup( task& task, const method& m )
{
    // averager mode
    assert( m.hor_.mode == 2 );
    
    auto inst = task.inst();
        
    ViInt32 int32Arg = ViInt32( m.hor_.nbrSamples );
    
    ViStatus status = AcqrsD1_configAvgConfig( inst, 0, "NbrSamples", &int32Arg );
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
device_ap240::protocol_setup( task& task, method& m )
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
device_ap240::readData( task& task, waveform& data )
{
    data.method_ = task.method();
    AqReadParameters readPar;
    AqDataDescriptor dataDesc;
    AqSegmentDescriptorAvg segDesc;
    memset(&readPar, 0, sizeof(readPar));
    memset(&dataDesc, 0, sizeof(dataDesc));
    memset(&segDesc, 0, sizeof(segDesc));

    data.d_.resize( task.method().hor_.nbrSamples + 32 );

    readPar.dataType = ReadInt32;
    readPar.readMode = ReadModeAvgW;
    readPar.nbrSegments = 1;
    readPar.nbrSamplesInSeg = task.method().hor_.nbrSamples;
    readPar.dataArraySize = ViInt32( data.d_.size() * sizeof( long ) );               // (nbrSamples + 32) * sizeof(ong)
    readPar.segDescArraySize = sizeof( AqSegmentDescriptorAvg );
    const int channel = 1;

    ViStatus st = AcqrsD1_readData( task.inst()
                                    , channel
                                    , &readPar
                                    , data.d_.data()
                                    , &dataDesc
                                    , &segDesc );
    
    task::checkError( task.inst(), st, "AcqrsD1_readData", __LINE__ );
    if ( dataDesc.indexFirstPoint > 0 )
        std::copy( data.d_.begin() + dataDesc.indexFirstPoint, data.d_.end() - 32, data.d_.begin() );
    
    data.d_.resize( data.d_.size() - 32 );

    std::cout << "## device_ap240::readData ##" << std::endl;

    double elapsed = double( static_cast<uint64_t>( segDesc.timeStampHi ) << 32 | segDesc.timeStampLo ) * 1.0e-12; // ps -> s

    std::cout << "actualTriggersInSeg: " << segDesc.actualTriggersInSeg
              << "\tsize: " << data.d_.size()
              << "\tindexFirstPoint: " << dataDesc.indexFirstPoint
              << "\tdelay: " << data.method_.hor_.delay
              << "\tX: " << elapsed
              << std::endl;

    //uint32_t nDelay = uint32_t( data.method_.digitizer_delay_to_first_sample * data.method_.samp_rate ) & ~0x1f;
    
    data.meta_.actualAverages = segDesc.actualTriggersInSeg;  // number of triggers for average
    data.meta_.actualPoints = dataDesc.returnedSamplesPerSeg; //data.d_.size();
    data.meta_.actualRecords = 0; // ??
    data.meta_.flags = segDesc.flags; // markers
    data.meta_.initialXOffset = dataDesc.sampTime * data.method_.hor_.nStartDelay;
    data.meta_.initialXTimeSeconds = elapsed;
    data.meta_.scaleFactor = dataDesc.vGain;
    data.meta_.scaleOffset = dataDesc.vOffset;
    data.meta_.xIncrement = dataDesc.sampTime;
        
    // avgr.sampInterval = static_cast<unsigned long>( scale_to_pico( desc.dataDesc.sampTime ) + 0.5 );
    // avgr.uptime = ( static_cast< unsigned long long>( segDesc.timeStampHi ) << 32 | segDesc.timeStampLo ) / 1000000LL; // us
    
    // avgr.nSamplingDelay = nStartDelay_;
    // avgr.avgrType = infitofinterface::Averager_AP240;
    // avgr.npos = data_serial_number_;
    // avgr.timeSinceInject = uint32_t( avgr.uptime - averager_inject_usec_ );
    // avgr.nbrSamples = nbrSamples_;
    // avgr.nbrAverage = uint32_t(nbrWaveforms_);
    
    // // 0:P1, 1:P2, 2:A, 3:B
    // avgr.wellKnownEvents = segDesc.flags & 0x0f;
    // if ( segDesc.flags & 02 ) {
    //     avgr.wellKnownEvents |= SignalObserver::wkEvent_INJECT;
    //     TOFTask::instance()->handle_event_in( avgr.wellKnownEvents );
    // }
    
    // data.meta_.actualAverages = actualAverages;
    // data.meta_.flags = saFlags.data()[ 0 ];
    // data.meta_.initialXTimeSeconds = saInitialXTimeSeconds.data()[ 0 ] + saInitialXTimeFraction.data()[ 0 ];
    // data.d_.resize( numPointsPerRecord );
    // std::copy( sa.data() + firstValidPoint, sa.data() + numPointsPerRecord, data.d_.begin() );
    return true;
}


