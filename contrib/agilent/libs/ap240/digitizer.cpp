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

#ifdef ERR
# undef ERR
#endif

#ifdef TERR
# undef TERR
#endif

#define ERR(e,m) do { adlog::logger(__FILE__,__LINE__,adlog::LOG_ERROR)<<e.Description()<<", "<<e.ErrorMessage(); error_reply(e,m); } while(0)
#define TERR(e,m) do { adlog::logger(__FILE__,__LINE__,adlog::LOG_ERROR)<<e.Description()<<", "<<e.ErrorMessage(); task.error_reply(e,m); } while(0)

namespace ap240 {

    class simulator;

    namespace detail {

        struct device_ap240 {
            static bool initial_setup( task&, const method& );
            static bool setup( task&, const method& );
            static bool acquire( task& );
            static bool waitForEndOfAcquisition( task&, int timeout );
            static bool readData( task&, waveform& );
        };

        struct device_simulator {
            static bool initial_setup( task&, const method& );
            static bool setup( task&, const method& );
            static bool acquire( task& );
            static bool waitForEndOfAcquisition( task&, int timeout );
            static bool readData( task&, waveform& );
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

            inline ap240::simulator * simulator() { return simulator_; }
            inline const ap240::method& method() const { return method_; }
            inline const identify& ident() const { return *ident_; }

            static std::string error_msg( int status, const char * ident ) {
                std::ostringstream o;
                ViChar errorMessage[ 512 ];
        
                if ( Acqrs_errorMessage( VI_NULL, status, errorMessage, sizeof( errorMessage ) ) == VI_SUCCESS ) {
                    o << errorMessage << " 0x" << std::hex << status << " at " << ident;
                } else {
                    o << "ERROR: code =0x" << std::hex << status << " at " << ident;
                }
                return o.str();
            }

            static bool checkError( ViSession instId, ViStatus st, const char * text, ViInt32 arg = 0 ) {
                std::cerr << text << " at " << arg << std::endl;
                if ( st == VI_SUCCESS )
                    return false;
                std::array< ViChar, 1024 > msg;
                std::fill( msg.begin(), msg.end(), 0 );
                AcqrsD1_errorMessageEx( instId, st, msg.data(), msg.size() );
                std::cerr << boost::format( "%s (0x%x): %s" ) % text % int(st) % msg.data();
                if ( arg )
                    std::cerr << " #" << arg;
                std::cerr << std::endl;
                //return true;
                return false;
            }
            inline ViSession inst() const { return inst_; }
            bool nbrSamples( size_t nbrSamples ) {

                ViInt32 int32Arg( static_cast<ViInt32>(nbrSamples) );
                ViStatus status = AcqrsD1_configAvgConfig( inst_, 0, "NbrSamples", &int32Arg );
                if ( status != VI_SUCCESS ) {
                    checkError( inst_, status, "AcqirisD1_configAvgConfig, NbrSamples", __LINE__  );
                    return false;
                } else
                    nbrSamples_ = int32Arg;
        
                return true;
            }

            bool nStartDelay( size_t nStartDelay ) {
                ViInt32 int32Arg( static_cast<ViInt32>(nStartDelay) );
        
                ViStatus res = AcqrsD1_configAvgConfig( inst_, 0, "StartDelay", &int32Arg );
                if ( checkError( inst_, res, "AcqrsD1_configAvgConfig StartDelay ", __LINE__ ) )
                    return false;
        
                nStartDelay_ = int32Arg;
                return true;
            }

            bool nbrWaveforms( size_t nbrWaveforms ) {
                ViInt32 int32Arg( static_cast<ViInt32>(nbrWaveforms) );
                ViStatus res = AcqrsD1_configAvgConfig( inst_, 0, "NbrWaveforms", &int32Arg );
                if ( checkError( inst_, res, "AcqrsD1_configAvgConfig NbrWaveforms ", int32Arg ) )
                    return false;
        
                nbrWaveforms_ = nbrWaveforms;
        
                return true;
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
            ap240::simulator * simulator_;
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

            std::deque< std::shared_ptr< ap240::SampleProcessor > > queue_;
            std::vector< digitizer::command_reply_type > reply_handlers_;
            std::vector< digitizer::waveform_reply_type > waveform_handlers_;
            std::shared_ptr< identify > ident_;
            std::shared_ptr< adportable::TimeSquaredScanLaw > scanlaw_;

            bool handle_initial_setup( int nDelay, int nSamples, int nAverage );
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
                    std::cout << boost::format( "Model '%1%', S/N:%2%, bus:slot=%3%:%4%" )
                        % model_name_ % serial_number_ % bus_number_ % slot_number_;
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

bool
digitizer::peripheral_prepare_for_run( const adcontrols::ControlMethod& m )
{
    using adcontrols::controlmethod::MethodItem;

    adcontrols::ControlMethod cm( m );
    cm.sort();
    auto it = std::find_if( cm.begin(), cm.end(), [] ( const MethodItem& mi ){ return mi.modelname() == "ap240"; } );
    if ( it != cm.end() ) {
        ap240::method m;
        if ( adportable::serializer< ap240::method >::deserialize( m, it->data(), it->size() ) ) {
            return task::instance()->prepare_for_run( m );
        }
    }
    return false;
}

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

const int32_t *
waveform::trim( metadata& meta, uint32_t& nSamples ) const
{
    meta = meta_;

    size_t offset = 0;
    if ( method_.digitizer_delay_to_first_sample < method_.delay_to_first_sample_ )
        offset = size_t( ( ( method_.delay_to_first_sample_ - method_.digitizer_delay_to_first_sample ) / meta.xIncrement ) + 0.5 );
    
    nSamples = method_.nbr_of_s_to_acquire_;
    if ( nSamples + offset > method_.digitizer_nbr_of_s_to_acquire )
        nSamples = uint32_t( method_.digitizer_nbr_of_s_to_acquire - offset );
    
    meta.initialXOffset = method_.delay_to_first_sample_;
    meta.actualPoints = nSamples;

    return d_.data() + offset;
}


////////////////////

task::task() : work_( io_service_ )
             , strand_( io_service_ )
             , simulated_( false )
             , simulator_( 0 )
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
    delete simulator_;
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
    io_service_.post( strand_.wrap( [&] { handle_initial_setup( 32, 1024 * 10, 2 ); } ) );
    return true;
}

bool
task::prepare_for_run( const ap240::method& m )
{
    ADTRACE() << "ap240::task::prepare_for_run";
    ADTRACE() << "\tfront_end_range: " << m.front_end_range << "\tfrontend_offset: " << m.front_end_offset
        << "\text_trigger_level: " << m.ext_trigger_level
        << "\tsamp_rate: " << m.samp_rate
        << "\tnbr_of_samples: " << m.nbr_of_s_to_acquire_ << "; " << m.digitizer_nbr_of_s_to_acquire
        << "\tnbr_of_average: " << m.nbr_of_averages
        << "\tdelay_to_first_s: " << adcontrols::metric::scale_to_micro( m.digitizer_delay_to_first_sample )
        << "\tinvert_signal: " << m.invert_signal
        << "\tnsa: " << m.nsa;
    
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
    // std::lock_guard< std::mutex > lock( mutex_ );
	if ( queue_.empty() ) {
        queue_.push_back( std::make_shared< SampleProcessor >( io_service_ ) );
        queue_.back()->prepare_storage( 0 );
    }
	// status_current_ = ControlServer::ePreparingForRun;
	// status_being_ = ControlServer::eReadyForRun;
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
task::handle_initial_setup( int nDelay, int nSamples, int nAverage )
{
    simulated_ = false;
    bool success = false;

    method_.samp_rate = 0.5e9;  // Hz
    method_.digitizer_nbr_of_s_to_acquire = 1600; // nSamples;  // number of samples per waveform
    method_.nStartDelay = 32; // nDelay;
    method_.delay_to_first_sample_ = double( nDelay ) / method_.samp_rate;
    method_.nbr_of_averages = 8;

    ViStatus status;
    ViStatus * pStatus = &status;

    if ( pStatus == 0 )
        pStatus = &status;

    std::cout << "AcqirisDxDir=" << getenv("AcqirisDxDir") << std::endl;

    if ( getenv("AcqirisDxDir") == 0 ) {
        std::cerr << L"AcqirisDxDir environment variable not set." << std::endl;
        return false;
    }
    
#ifdef _LINUX
    struct stat st;
    if ( stat( "/dev/acqrsPCI", &st ) != 0 ) {
        std::cerr << L"/dev/acqrsPID does not exists" << std::endl;
        return false;
    }
#endif
    std::cerr << "found /dev/acqrsPCI" << std::endl;

    if ( ( status = AcqrsD1_multiInstrAutoDefine( "cal=0", &numInstruments_ ) ) != VI_SUCCESS ) {
        std::cerr << error_msg( status, "Acqiris::findDevice()" ) << std::endl;
    } else {
        std::cout << boost::format( "find %1% acqiris devices." ) % numInstruments_ << std::endl;
    }
    
    if ( numInstruments_ == 0 )
        return false;

    for ( int i = 0; i < numInstruments_; ++i ) {
        device_name_ = ( boost::format( "PCI::INSTR%1%" ) % i ).str();
        inst_ = (-1);
        status = Acqrs_init( const_cast< char *>(device_name_.c_str()), VI_FALSE, VI_FALSE, &inst_);
        if ( inst_ != ViSession(-1) && getInstrumentData() ) {
            std::cerr << "\tfound device on: " << device_name_ << std::endl;
            success = true;
            break;
        } else {
            std::cerr << error_msg( status, "Acqiris::findDevice" ) << std::endl;
        }
    }
    
    // for ( auto& reply: reply_handlers_ ) reply( "Identifier", ident_->Identifier );
    // for ( auto& reply: reply_handlers_ ) reply( "Revision", ident_->Revision );
    // for ( auto& reply: reply_handlers_ ) reply( "Description", ident_->Description );
    // for ( auto& reply: reply_handlers_ ) reply( "InstrumentModel", ident_->InstrumentModel );
    // for ( auto& reply: reply_handlers_ ) reply( "InstrumentFirmwareRevision", ident_->FirmwareRevision );
    // for ( auto& reply: reply_handlers_ ) reply( "SerialNumber", ident_->SerialNumber );
    // for ( auto& reply: reply_handlers_ ) reply( "IOVersion", ident_->IOVersion );
    // for ( auto& reply: reply_handlers_ ) reply( "Options", ident_->Options );
    
    if ( simulated_ )
        device_simulator::initial_setup( *this, method_ );
    else
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
    if ( simulated_ )
        device_simulator::initial_setup( *this, m );
    else
        device_ap240::initial_setup( *this, m );
    method_ = m;
    return true;
}

bool
task::handle_protocol( const ap240::method m )
{
#if defined _DEBUG
    ADTRACE() << "ap240::task::handle_protocol"
        << "\tnbr_of_samples: " << m.digitizer_nbr_of_s_to_acquire
        << "\tnbr_of_average: " << m.nbr_of_averages
        << "\tdelay_to_first_s: " << adcontrols::metric::scale_to_micro( m.digitizer_delay_to_first_sample );
#endif
    if ( simulated_ )
        device_simulator::setup( *this, m );
    else
        device_ap240::setup( *this, m );
    method_ = m;
    return true;
}


bool
task::handle_acquire()
{
    static int counter_;

    ++acquire_post_count_;
    io_service_.post( strand_.wrap( [&] { handle_acquire(); } ) );    // scedule for next acquire

    --acquire_post_count_;
    if ( acquire() ) {
        if ( waitForEndOfAcquisition( 3000 ) ) {
            auto avgr = std::make_shared< waveform >( ident_ );
            if ( readData( *avgr ) ) {
                // if ( software_events_ ) {
                //     avgr->wellKnownEvents |= software_events_; // marge with hardware events
                //     software_events_ = 0;

                //     // set time for injection to zero (a.k.a. retention time)
                //     if ( avgr->wellKnownEvents & SignalObserver::wkEvent_INJECT ) {
                //         averager_inject_usec_ = avgr->uptime;
                //         avgr->timeSinceInject = 0;
                //     }
                // }
                // assert( avgr->nbrSamples );
                ap240::method m;
                for ( auto& reply: waveform_handlers_ ) {
                    if ( reply( avgr.get(), m ) )
                        handle_protocol( m );
                }
            }
        } else {
            ADTRACE() << "===== handle_acquire waitForEndOfAcquisitioon == not handled.";
        }
        return true;
    }
    return false;
}

bool
task::acquire()
{
    if ( simulated_ )
        return device_simulator::acquire( *this );
    else
        return device_ap240::acquire( *this );
}

bool
task::waitForEndOfAcquisition( int timeout )
{
    if ( simulated_ )
        return device_simulator::waitForEndOfAcquisition( *this, timeout );
    else
        return device_ap240::waitForEndOfAcquisition( *this, timeout );
}

bool
task::readData( waveform& data )
{
    data.serialnumber_ = data_serialnumber_++;
    if ( simulated_ )
        return device_simulator::readData( *this, data );
    else
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

identify::identify( const identify& t ) : Identifier( t.Identifier )
                                        , Revision( t.Revision )
                                        , Vendor( t.Vendor )
                                        , Description( t.Description )
                                        , InstrumentModel( t.InstrumentModel )
                                        , FirmwareRevision( t.FirmwareRevision )
                                        , SerialNumber( t.SerialNumber )
                                        , Options( t.Options )
                                        , IOVersion( t.IOVersion )
                                        , NbrADCBits( t.NbrADCBits )
                                        , bus_number_(t.bus_number_)
                                        , slot_number_(t.slot_number_)
                                        , serial_number_(t.serial_number_)
{
}

bool
device_ap240::initial_setup( task& task, const method& m )
{
    ViStatus status;
    ViStatus * pStatus = &status;

    std::cout << "######### device_ap240::initial_setup #############" << std::endl;

    int nDelay = m.nStartDelay;
    int nSamples = 1600;
    int nAverage = 4;

    if ( pStatus == 0 )
        pStatus = &status;
	
    //                            ch = 1, fs=1.0V, offset = 0.0v, coupling = DC 50ohm, bw = 700MHz
    status = AcqrsD1_configVertical( task.inst(), 1, 1.0, 0.0, 3, 2 );
    if ( task::checkError( task.inst(), status, "configVertical", __LINE__ ) )
        return false;

    // External trig. input
    status = AcqrsD1_configVertical( task.inst(), -1, 5.0, 0.0, 3 /*DC 50ohm*/, 0 /* no bw */);
    if ( task::checkError( task.inst(), status, "configVertical (2)", __LINE__ ) )
        return false;
    
    status = AcqrsD1_configMemory( task.inst(), nSamples, 1 );
    if ( task::checkError( task.inst(), status, "configMemory", __LINE__ ) )
        ; //return false;

    status = AcqrsD1_configTrigClass( task.inst(), 0, 0x80000000, 0, 0, 0, 0 );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configTrigClass", __LINE__  ) )
        return false;
	
    // ExtTrigSource(-1), DC coupling(0), positive(0), 1000mV (1400)
    status = AcqrsD1_configTrigSource( task.inst(), -1, 0, 0, 500, 0 );  
    if ( task::checkError( task.inst(), status, "AcqrsD1_configTrigSource", __LINE__  ) )
        return false;
	
    status = AcqrsD1_configMode( task.inst(), 2, 0, 0 ); // 2 := averaging mode, 0 := normal data acq.
    if ( task::checkError( task.inst(), status, "AcqrsD1_configMode", __LINE__  ) )
        return false;
	
    status = AcqrsD1_configMultiInput( task.inst(), 1, 0 );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configMultiInput", __LINE__  ) )
        return false;

    status = AcqrsD1_configChannelCombination( task.inst(), 2, 1 );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configChannelCombination", __LINE__  ) )
        return false;

#if 0
    // config "IO A" -- it seems not working for input level
    status = AcqrsD1_configControlIO( task.inst(), 1, 1, 0, 0 );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configControlIO(A)" ) )
         return false;
#endif
    // config "IO B" for Acquisition is active (21)
    status = AcqrsD1_configControlIO( task.inst(), 2, 21, 0, 0 );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configControlIO(B)", __LINE__  ) )
        return false;

    // Configure the front panel trigger out (TR.)
    // The appropriate offset is 1,610 mV.
    status = AcqrsD1_configControlIO( task.inst(), 9, 1610 / 2, 0, 0 );
    //status = AcqrsD1_configControlIO( task.inst(), 9, 0, 0, 0 );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configControlIO (2)", __LINE__  ) )
        return false;

    if ( ! task::instance()->nbrSamples( nSamples ) ) //	if ( ! nbrSamples( 73728 ) )
        return false;

    ViInt32 int32Arg;
	
    if ( ! task::instance()->nStartDelay( nDelay ) )  //34048
        return false;

    // "P2Control" set to average(out) --> disable for debug
    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( task.inst(), 0, "P1Control", &int32Arg );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configAvgConfig (P1Control)", __LINE__  ) )
        return false;

    // "P2Control" to disable
	int32Arg = 0;
    status = AcqrsD1_configAvgConfig( task.inst(), 0, "P2Control", &int32Arg );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configAvgConfig (P2Control)", __LINE__  ) )
        return false;

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( task.inst(), 0, "DitherRange", &int32Arg );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configAvgConfig (DitherRange)", __LINE__  ) )
        return false;
	
    int32Arg = 1;
    status = AcqrsD1_configAvgConfig( task.inst(), 0, "NbrSegments", &int32Arg );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configAvgConfig (NbrSegments)", __LINE__  ) )
        return false;

    if ( ! task.nbrWaveforms( nAverage ) )
        return false;

    int32Arg = 0;
    *pStatus = AcqrsD1_configAvgConfig( task.inst(), 0, "StopDelay", &int32Arg );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configAvgConfig (StopDelay)", __LINE__  ) )
        return false;

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( task.inst(), 0, "TrigAlways", &int32Arg );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configAvgConfig (TrigAlways)", __LINE__  ) )
        return false;

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( task.inst(), 0, "TrigResync", &int32Arg );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configAvgConfig (TrigResync)", __LINE__  ) )
        return false;

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( task.inst(), 0, "ThresholdEnable", &int32Arg );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configAvgConfig (ThresholdEnable)", __LINE__  ) )
        return false;

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( task.inst(), 0, "TriggerTimeout", &int32Arg );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configAvgConfig (TriggerTimeut)", __LINE__  ) )
        return false;

    status = AcqrsD1_configHorizontal( task.inst(), 0.5e-9, 0 );
    if ( task::checkError( task.inst(), status, "AcqrsD1_configHorizontal", __LINE__  ) )
        return false;

    //-----------------------
    do {
        int32Arg = 1;
        status = AcqrsD1_configAvgConfig( task.inst(), 0, "TimestampClock", &int32Arg);
        if ( task::checkError( task.inst(), status, "AcqrsD1_configAvgConfig (TimestampClock)", __LINE__  ) )
            return false;
    } while (0);
	
    do {
        int32Arg = 1;  // Always on first trigger
        status = AcqrsD1_configAvgConfig( task.inst(), 0, "MarkerLatchMode", &int32Arg); 
        if ( task::checkError( task.inst(), status, "AcqrsD1_configAvgConfig (MarkerLatchMode)", __LINE__  ) )
            return false;
    } while (0);

    do {
        int32Arg = 1; // invert data for mass spectrum;
        status = AcqrsD1_configAvgConfig( task.inst(), 0, "InvertData", &int32Arg); 
        if ( task::checkError( task.inst(), status, "AcqrsD1_configAvgConfig (InvertData)", __LINE__  ) )
            return false;
    } while (0);
	
    return true;
}

bool
device_ap240::setup( task& task, const method& m )
{
    return true; //device_ap240::initial_setup( task, m );
}

bool
device_ap240::acquire( task& task )
{
    std::cout << "######### device_ap240::acquire #############" << std::endl;
    return AcqrsD1_acquire( task.inst() ) == VI_SUCCESS;
}

bool
device_ap240::waitForEndOfAcquisition( task& task, int timeout )
{
    std::cout << "######### device_ap240::waitForEndOfAcquisition #############" << std::endl;    
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
    std::cout << "######### device_ap240::readData #############" << std::endl;
    
    data.method_ = task.method();
    AqReadParameters readPar;
    AqDataDescriptor dataDesc;
    AqSegmentDescriptorAvg segDesc;
    memset(&readPar, 0, sizeof(readPar));
    memset(&dataDesc, 0, sizeof(dataDesc));
    memset(&segDesc, 0, sizeof(segDesc));

    data.d_.resize( task.method().digitizer_nbr_of_s_to_acquire + 32 );

    readPar.dataType = ReadInt32;
    readPar.readMode = ReadModeAvgW;
    readPar.nbrSegments = 1;
    readPar.nbrSamplesInSeg = task.method().digitizer_nbr_of_s_to_acquire;
    readPar.dataArraySize = data.d_.size() * sizeof( long );
    readPar.segDescArraySize = sizeof( AqSegmentDescriptorAvg );
    const int channel = 1;
    ViStatus st = AcqrsD1_readData( task.inst()
                                   , channel
                                   , &readPar
                                   , data.d_.data()
                                   , &dataDesc
                                   , &segDesc );

    task::checkError( task.inst(), st, "AcqrsD1_readData", 0 );
    data.meta_.actualAverages = segDesc.actualTriggersInSeg;
    data.meta_.actualPoints = data.d_.size();
    data.meta_.actualRecords = 0; // ??
    data.meta_.flags = segDesc.flags; // markers
    data.meta_.initialXOffset = dataDesc.sampTime * ( data.method_.nStartDelay - dataDesc.indexFirstPoint );
    data.meta_.initialXTimeSeconds = double( static_cast<uint64_t>( segDesc.timeStampHi ) << 32LL + segDesc.timeStampLo ) * 1.0e-12; // ps -> s
    data.meta_.scaleFactor = 1.0;
    data.meta_.scaleOffset = 0;
    data.meta_.xIncrement = 0.5e-9;
    
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


bool
device_simulator::initial_setup( task& task, const method& m )
{
	return true;
}

bool
device_simulator::setup( task& task, const method& m )
{
    return true; // device_simulator::initial_setup( task, m );
}

bool
device_simulator::acquire( task& task )
{
    return false;
}

bool
device_simulator::waitForEndOfAcquisition( task& task, int /* timeout */)
{
    return false;
}

bool
device_simulator::readData( task& task, waveform& data )
{
    return false;
}

