/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "simulator.hpp"
#include "agmd2.hpp"
#include "automaton.hpp"
#include <acqrscontrols/u5303a/method.hpp>
#include <adlog/logger.hpp>
#include <adicontroller/constants.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/mblock.hpp>
#include <adportable/serializer.hpp>
#include <adportable/string.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <workaround/boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/type_traits.hpp>
#include <boost/variant.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

namespace u5303a {

    namespace detail {
 
        class task : public fsm::handler {
            task();
            ~task();
        public:
            static task * instance();
            static const std::chrono::steady_clock::time_point uptime_;
            static const uint64_t tp0_;
            std::exception_ptr exptr_;
            
            inline boost::asio::io_service& io_service() { return io_service_; }
            
            void terminate();
            bool initialize();
            bool prepare_for_run( const adcontrols::ControlMethod::Method& );
            bool prepare_for_run( const acqrscontrols::u5303a::method& );
            bool run();
            bool stop();
            bool trigger_inject_out();

            void connect( digitizer::command_reply_type f );
            void disconnect( digitizer::command_reply_type f );
            void connect( digitizer::waveform_reply_type f );
            void disconnect( digitizer::waveform_reply_type f );
            void setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw >& ptr );

            bool findResource();

            inline AgMD2 * spDriver() { return spDriver_.get(); }

            inline const acqrscontrols::u5303a::method& method() const { return method_; }

            inline const acqrscontrols::u5303a::identify& ident() const { return *ident_; }

            inline std::shared_ptr< acqrscontrols::u5303a::identify > ident_ptr() { return ident_; }

            inline bool isSimulated() const { return simulated_; }

            inline const std::chrono::steady_clock::time_point& tp_acquire() const { return tp_acquire_; }
            
            void error_reply( const std::string& emsg, const std::string& );

        private:
            // fsm::handler
            void fsm_action_prepare() override;
            void fsm_action_stop() override;
            void fsm_action_TSR_stop() override;
            void fsm_action_initiate() override;
            void fsm_action_TSR_initiate() override;
            void fsm_action_continue() override;
            void fsm_action_TSR_continue() override;
            void fsm_state( bool, fsm::idState ) override;
            
        private:
            friend std::unique_ptr< task >::deleter_type;

            static std::mutex mutex_;

            uint64_t digitizerNumRecords_;

            fsm::controller fsm_;
            std::shared_ptr< AgMD2 > spDriver_;
            std::vector< adportable::asio::thread > threads_;
            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;
            boost::asio::io_service::strand strand_;
            bool simulated_;
            acqrscontrols::u5303a::method method_;
            std::atomic_flag acquire_posted_;   // only one 'acquire' handler can be in the strand
            
            bool c_injection_requested_; 
            bool c_acquisition_status_; // true: acq. is active, indeterminant: inj. waiting, 
            std::atomic<double> u5303_inject_timepoint_;
            std::vector< std::string > foundResources_;
            std::vector< digitizer::command_reply_type > reply_handlers_;
            std::vector< digitizer::waveform_reply_type > waveform_handlers_;
            std::shared_ptr< acqrscontrols::u5303a::identify > ident_;
            std::shared_ptr< adportable::TimeSquaredScanLaw > scanlaw_;
            std::chrono::steady_clock::time_point tp_acquire_;

            bool handle_initial_setup();
            bool handle_terminating();
            bool handle_acquire();
            bool handle_TSR_acquire();
            bool handle_prepare_for_run( const acqrscontrols::u5303a::method );
            bool handle_protocol( const acqrscontrols::u5303a::method );            
            bool acquire();
            bool waitForEndOfAcquisition( int timeout );
            bool readData( acqrscontrols::u5303a::waveform& );
            void set_time_since_inject( acqrscontrols::u5303a::waveform& );
        };

        struct device {
            static bool initial_setup( task&, const acqrscontrols::u5303a::method&, const std::string& options );
            static bool setup( task&, const acqrscontrols::u5303a::method& );
            static bool acquire( task& );
            static bool waitForEndOfAcquisition( task&, int timeout );
        };

        const std::chrono::steady_clock::time_point task::uptime_ = std::chrono::steady_clock::now();
        const uint64_t task::tp0_ = std::chrono::duration_cast<std::chrono::nanoseconds>( task::uptime_.time_since_epoch() ).count();

    }
}

using namespace u5303a;
using namespace u5303a::detail;

//std::unique_ptr< task > task::instance_;

std::mutex task::mutex_;

digitizer::digitizer()
{
    boost::interprocess::shared_memory_object::remove( "waveform_simulator" );
}

digitizer::~digitizer()
{
}

bool
digitizer::peripheral_initialize()
{
    return task::instance()->initialize();
}

bool
digitizer::peripheral_prepare_for_run( const adcontrols::ControlMethod::Method& m )
{
    using adcontrols::ControlMethod::MethodItem;

    adcontrols::ControlMethod::Method cm( m );
    cm.sort();
    auto it = std::find_if( cm.begin(), cm.end(), [] ( const MethodItem& mi ){ return mi.modelname() == "u5303a"; } );
    if ( it != cm.end() ) {
        acqrscontrols::u5303a::method m;

        if ( it->get( *it, m ) ) {
            return task::instance()->prepare_for_run( m );
        }

    }
    return false;
}

bool
digitizer::peripheral_prepare_for_run( const acqrscontrols::u5303a::method& m )
{

    return task::instance()->prepare_for_run( m );
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
digitizer::peripheral_terminate()
{
    task::instance()->terminate();
    return true;
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

task::task() : work_( io_service_ )
             , strand_( io_service_ )
             , simulated_( false )
             , exptr_( nullptr )
             , digitizerNumRecords_( 1 )
             , fsm_( this )
             , c_injection_requested_( false )
             , c_acquisition_status_( false )
             , u5303_inject_timepoint_( 0 )
{
    acquire_posted_.clear();
    
    fsm_.start(); // Stopped state

    threads_.push_back( adportable::asio::thread( [this]() {
                try {
                    io_service_.run();
                } catch ( ... ) {
                    ADERROR() << "Exception: " << boost::current_exception_diagnostic_information();
                    exptr_ = std::current_exception();
                }
            } ) );
    
}

task::~task()
{
    ADDEBUG() << "******** task dtor";
}

bool
task::findResource()
{
    // workaround
    foundResources_.clear();
    foundResources_.push_back( "PXI3::0::0::INSTR" );
    foundResources_.push_back( "PXI4::0::0::INSTR" );
    foundResources_.push_back( "PXI5::0::0::INSTR" );
    foundResources_.push_back( "PXI0::0::0::INSTR" );
    foundResources_.push_back( "PXI1::0::0::INSTR" );
    return true;
}

task *
task::instance()
{
    static task __task__;
    return &__task__;
}

bool
task::initialize()
{
    ADTRACE() << "u5303a digitizer initializing...";

	io_service_.post( strand_.wrap( [this] { findResource(); } ) );

    io_service_.post( strand_.wrap( [this] { handle_initial_setup(); } ) );
        
    return true;
}

bool
task::prepare_for_run( const acqrscontrols::u5303a::method& method )
{
    auto& m = method.method_;

#if 0
    ADDEBUG() << "u5303a::task::prepare_for_run";
    ADDEBUG() << "\tfront_end_range: " << m.front_end_range << "\tfrontend_offset: " << m.front_end_offset
              << "\text_trigger_level: " << m.ext_trigger_level
              << "\tsamp_rate: " << m.samp_rate
              << "\tnbr_of_samples: " << m.nbr_of_s_to_acquire_ << "; " << m.digitizer_nbr_of_s_to_acquire
              << "\tnbr_of_average: " << m.nbr_of_averages
              << "\tdelay_to_first_s: " << adcontrols::metric::scale_to_micro( m.digitizer_delay_to_first_sample )
              << "\tinvert_signal: " << m.invert_signal
              << "\tnsa: " << m.nsa;
#endif

    io_service_.post( strand_.wrap( [=] { handle_prepare_for_run( method ); } ) );

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
    fsm_.process_event( fsm::Stop() );
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
    // if ( spDriver_ ) 
    //     spDriver_->Abort();

    io_service_.stop();
    
    for ( std::thread& t: threads_ )
        t.join();

    threads_.clear();    
}


///////////////////////////////////////
//////////////////////////////////////

void
task::fsm_state( bool enter, fsm::idState state )
{
    if ( state == fsm::idStopped && enter ) {
        for ( auto& reply: reply_handlers_ )
            reply( "StateChanged", "Stopped" );
    } else if ( state == fsm::idReadyToInitiate && enter ) {
        for ( auto& reply: reply_handlers_ )
            reply( "StateChanged", "Running" );
    }
}

void
task::fsm_action_stop()
{
    // Non TSR stop
}

void
task::fsm_action_TSR_stop()
{
    spDriver_->Abort();
}

void
task::fsm_action_prepare()
{

    if ( spDriver_->TSREnabled() )
        spDriver_->Abort();
}

void
task::fsm_action_initiate()
{
    if ( ! acquire_posted_.test_and_set() ) {
        io_service_.post( strand_.wrap( [this] { handle_acquire(); } ) );
    }
}

void
task::fsm_action_TSR_initiate()
{
    if ( spDriver_->isIdle() )
        acquire();

    if ( ! acquire_posted_.test_and_set() ) {

        io_service_.post( strand_.wrap( [this] { handle_TSR_acquire(); } ) );

    }
}

void
task::fsm_action_continue()
{
    if ( ! acquire_posted_.test_and_set() ) {
        io_service_.post( strand_.wrap( [this] { handle_acquire(); } ) );
    }
}

void
task::fsm_action_TSR_continue()
{
    if ( ! acquire_posted_.test_and_set() ) {
        io_service_.post( strand_.wrap( [this] { handle_TSR_acquire(); } ) );
    }
}

void
task::set_time_since_inject( acqrscontrols::u5303a::waveform& waveform )
{
    if ( c_injection_requested_ ) {
        
        c_injection_requested_ = false;
        c_acquisition_status_ = true;
        u5303_inject_timepoint_ = waveform.meta_.initialXTimeSeconds;
        waveform.wellKnownEvents_ |= adicontroller::SignalObserver::wkEvent_INJECT;
    }

    waveform.timeSinceInject_ = waveform.meta_.initialXTimeSeconds - u5303_inject_timepoint_;
    if ( c_acquisition_status_ )
        waveform.wellKnownEvents_ |= adicontroller::SignalObserver::wkEvent_AcqInProgress;
}

bool
task::handle_initial_setup()
{
    spDriver_ = std::make_shared< AgMD2 >();

    ViBoolean idQuery = VI_TRUE;
    ViBoolean reset   = VI_TRUE;
    bool simulated = false;
    bool success = false;

    const char * strInitOptions = "Simulate=false, DriverSetup= Model=U5303A";

    if ( auto p = getenv( "AcqirisOption" ) ) {
        if ( p && std::strcmp( p, "simulate" ) == 0 ) {
            strInitOptions = "Simulate=true, DriverSetup= Model=U5303A";
            simulated = true;
            success = spDriver_->InitWithOptions( "PXI40::0::0::INSTR", VI_TRUE, VI_TRUE, strInitOptions );
        }
    }

    if ( !simulated ) {
        for ( auto& res : foundResources_ ) {
            ADTRACE() << "Initialize resource: " << res;
            if ( success = spDriver_->InitWithOptions( res.c_str(), VI_TRUE, VI_TRUE, strInitOptions ) )
                break;
        }
    }

    if ( success ) {
        simulated_ = simulated;
        
        ident_ = std::make_shared< acqrscontrols::u5303a::identify >();
        spDriver_->Identify( ident_ );
        // SR0 = 0.5GS/s 2ch; SR0+INT = 1.0GS/s 1ch;
        // SR1 = 1.0GS/s 2ch; SR1+INT = 2.0GS/s 1ch;
        // SR2 = 1.6GS/s 2ch; SR2+INT = 3.2GS/s 1ch;
        // M02 = 256MB; M10 = 1GB, M40 = 4GB
        
        for ( auto& reply : reply_handlers_ ) reply( "Identifier", ident_->Identifier() );
        for ( auto& reply : reply_handlers_ ) reply( "Revision", ident_->Revision() );
        for ( auto& reply : reply_handlers_ ) reply( "Description", ident_->Description() );
        for ( auto& reply : reply_handlers_ ) reply( "InstrumentModel", ident_->InstrumentModel() );
        for ( auto& reply : reply_handlers_ ) reply( "InstrumentFirmwareRevision", ident_->FirmwareRevision() );
        for ( auto& reply : reply_handlers_ ) reply( "SerialNumber", ident_->SerialNumber() );
        for ( auto& reply : reply_handlers_ ) reply( "IOVersion", ident_->IOVersion() );
        for ( auto& reply : reply_handlers_ ) reply( "Options", ident_->Options() );

        device::initial_setup( *this, method_, ident().Options() );
    }

    for ( auto& reply: reply_handlers_ )
        reply( "InitialSetup", ( success ? "success" : "failed" ) );

    spDriver_->Abort();
    fsm_.process_event( fsm::Stop() );

	return success;
}

bool
task::handle_terminating()
{
	return false;
}

bool
task::handle_prepare_for_run( const acqrscontrols::u5303a::method m )
{
    fsm_.process_event( fsm::Prepare() );

    c_acquisition_status_ = false;
    c_injection_requested_ = false;
    u5303_inject_timepoint_ = 0;

    device::initial_setup( *this, m, ident().Options() );

    if ( /* m.mode_ && */ simulated_ ) {
        acqrscontrols::u5303a::method a( m );
        a.method_.samp_rate = spDriver()->SampleRate();
        simulator::instance()->setup( a );
    }

    method_ = m;
    
    if ( m.method_.TSR_enabled ) {
        fsm_.process_event( fsm::TSRInitiate() );
    } else {
        fsm_.process_event( fsm::Initiate() );
    }

    return true;
}

bool
task::handle_protocol( const acqrscontrols::u5303a::method m )
{
    if ( m.mode_ == 0 )
        device::setup( *this, m );
    else
        device::setup( *this, m );

    if ( /* m.mode_ && */ simulated_ )
        simulator::instance()->setup( m );
    
    method_ = m;
    return true;
}


bool
task::handle_TSR_acquire()
{
    acquire_posted_.clear();
    fsm_.process_event( fsm::Continue() );
    
    if ( spDriver()->TSRMemoryOverflowOccured() )
        ADTRACE() << "Memory Overflow";

    auto tp = std::chrono::steady_clock::now() + std::chrono::milliseconds( 1000 ); // wait for max 1 second

    boost::tribool complete;
    while ( ! ( complete = spDriver()->isTSRAcquisitionComplete() ) && ( std::chrono::steady_clock::now() < tp ) ) {
        std::this_thread::sleep_for( std::chrono::microseconds( 1000 ) ); // assume 1ms trig. interval
    }

    if ( !complete )
        return false;
    
    std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > > vec;
    
    digitizer::readData( *spDriver(), method_, vec );
    spDriver_->TSRContinue();    

    for ( auto& waveform: vec ) {
        // ==> set elapsed time for debugging 
        set_time_since_inject( *waveform );
        // <==
        acqrscontrols::u5303a::method m;
        for ( auto& reply: waveform_handlers_ ) {
            if ( reply( waveform.get(), nullptr, m ) )
                handle_protocol( m );
        }
    }
    return true;
}

bool
task::handle_acquire()
{
    acquire_posted_.clear();  // make sure only one 'acquire' handler is in the strand que
    
    fsm_.process_event( fsm::Continue() );
    
    if ( acquire() ) {

        if ( waitForEndOfAcquisition( 3000 ) ) {
            if ( method_.mode_ == 0 ) { // digitizer

                std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > > vec;
                digitizer::readData( *spDriver(), method_, vec );

                if ( simulated_ )
                    simulator::instance()->touchup( vec );

                for ( auto& waveform: vec ) {
                    // ==> set elapsed time for debugging 
                    set_time_since_inject( *waveform );
                    // <==

                    acqrscontrols::u5303a::method m;
                    for ( auto& reply: waveform_handlers_ ) {
                        if ( reply( waveform.get(), nullptr, m ) )
                            handle_protocol( m );
                    }
                }
            } else { // average
                uint32_t events( 0 );
                auto waveform = std::make_shared< acqrscontrols::u5303a::waveform >( ident_, events );
                if ( readData( *waveform ) ) {
                    // ==> set elapsed time for debugging 
                    set_time_since_inject( *waveform );
                    // <==
                    acqrscontrols::u5303a::method m;
                    for ( auto& reply : waveform_handlers_ ) {
                        if ( reply( waveform.get(), nullptr, m ) )
                            handle_protocol( m );
                    }
                }
            }
            return true;
        }
    }
    return false;
}

bool
task::acquire()
{
    if ( method_.mode_ && simulated_ )    
        return simulator::instance()->acquire();

    tp_acquire_ = std::chrono::steady_clock::now();
    return device::acquire( *this );
}

bool
task::waitForEndOfAcquisition( int timeout )
{
    if ( simulated_ ) {
#if defined _MSC_VER && defined _DEBUG
        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
#else
        std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
#endif
        if ( method_.mode_ )
            return simulator::instance()->waitForEndOfAcquisition();
    }

    return device::waitForEndOfAcquisition( *this, timeout );
}

bool
task::readData( acqrscontrols::u5303a::waveform& data )
{
    data.serialnumber_ = spDriver()->dataSerialNumber();

    if ( simulated_ ) {
        if ( simulator::instance()->readData( data ) ) {
            data.timeSinceEpoch_ = std::chrono::steady_clock::now().time_since_epoch().count();
            // ==> set elapsed time for debugging 
            set_time_since_inject( data );
            // <==
            return true;
        }
    }
    //-------------------

    bool result( false );
    if ( method_.mode_ == 0 ) {
        result = digitizer::readData16( *spDriver(), method_, data );
    } else {
        result = digitizer::readData32( *spDriver(), method_, data );
    }
    // ==> set elapsed time for debugging 
    set_time_since_inject( data );
    // <==
    return result;
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
    // no way to do 'disconnect' since std::function<> has no operator == implemented.
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
    // no way to do 'disconnect' since std::function<> has no operator == implemented.
}

void
task::error_reply( const std::string& e, const std::string& method )
{
    for ( auto& reply: reply_handlers_ )
        reply( method, e.c_str() );
}

void
task::setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw >& ptr )
{
    scanlaw_ = ptr;
}

bool
device::initial_setup( task& task, const acqrscontrols::u5303a::method& m, const std::string& options )
{
#if defined _MSC_VER
    ViInt32 const coupling = AGMD2_VAL_VERTICAL_COUPLING_DC;
#else
    ViInt32 constexpr coupling = AGMD2_VAL_VERTICAL_COUPLING_DC;    
#endif

    if ( options.find( "INT" ) != options.npos ) {
        task.spDriver()->ConfigureTimeInterleavedChannelList( "Channel1", "Channel2" );
    }

    task.spDriver()->log(
        AgMD2_ConfigureChannel( task.spDriver()->session(), "Channel1", m.method_.front_end_range, m.method_.front_end_offset, coupling, VI_TRUE )
        , __FILE__, __LINE__ );
    task.spDriver()->setActiveTriggerSource( "External1" );
    task.spDriver()->setTriggerLevel( "External1", m.method_.ext_trigger_level );
    task.spDriver()->setTriggerSlope( "External1", AGMD2_VAL_POSITIVE );
    task.spDriver()->setTriggerCoupling( "External1", AGMD2_VAL_TRIGGER_COUPLING_DC );
    task.spDriver()->setTriggerDelay( m.method_.digitizer_delay_to_first_sample );
    // task.spDriver()->setTriggerHoldOff( 1.0e-6 ); // 1us

    bool success = false;
    
    double max_rate = 3.2e9;
    if ( options.find( "SR1" ) != options.npos ) {
        max_rate = 1.0e9;
        if ( options.find( "INT" ) != options.npos )
            max_rate = 2.0e9;
    } else if ( options.find( "SR2" ) != options.npos ) {
        max_rate = 1.6e9;
        if ( options.find( "INT" ) != options.npos )
            max_rate = 3.2e9;
    }
    
    for ( auto samp_rate : { m.method_.samp_rate, max_rate } ) {
        if ( success = task.spDriver()->setSampleRate( samp_rate ) )
            break;
    }
        
    if ( m.mode_ == 0 ) { // Digitizer 
        // ADDEBUG() << "Normal Mode";
        task.spDriver()->setTSREnabled( m.method_.TSR_enabled );
        task.spDriver()->setAcquisitionMode( AGMD2_VAL_ACQUISITION_MODE_NORMAL );
        task.spDriver()->setAcquisitionRecordSize( m.method_.digitizer_nbr_of_s_to_acquire );
        task.spDriver()->setAcquisitionNumRecordsToAcquire( m.method_.nbr_records );

    } else { // Averager

        // ADDEBUG() << "Averager Mode";
        task.spDriver()->setTSREnabled( false );
        task.spDriver()->setDataInversionEnabled( "Channel1", m.method_.invert_signal ? true : false );
        task.spDriver()->setAcquisitionRecordSize( m.method_.digitizer_nbr_of_s_to_acquire );
        task.spDriver()->setAcquisitionNumRecordsToAcquire( 1 );
        task.spDriver()->setAcquisitionNumberOfAverages( m.method_.nbr_of_averages );

        // It looks like this command should be issued as last
        task.spDriver()->setAcquisitionMode( AGMD2_VAL_ACQUISITION_MODE_AVERAGER );
    }

    // ADTRACE() << "##### ACQUISITION_MODE : " << task.spDriver()->AcquisitionMode();
    
    task.spDriver()->CalibrationSelfCalibrate();

	return true;
}

bool
device::setup( task& task, const acqrscontrols::u5303a::method& m )
{
    // Originally, this was considered to use 'protocol' acquisition, which change acquisition parameter for each scan
    // However, acquisition parameter change require self-calibration on U5303A so that this can't be used.
    return true;
}

bool
device::acquire( task& task )
{
    return task.spDriver()->AcquisitionInitiate();
}

bool
device::waitForEndOfAcquisition( task& task, int timeout )
{
	(void)timeout;

    long const timeoutInMs = 3000;
    return task.spDriver()->AcquisitionWaitForAcquisitionComplete(timeoutInMs);
}

/////////////
bool
digitizer::readData( AgMD2& md2, const acqrscontrols::u5303a::method& m, std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > >& vec )
{
    ViInt64 arraySize = 0;
    const int64_t recordSize = m.method_.digitizer_nbr_of_s_to_acquire;
    const int64_t numRecords = m.method_.nbr_records;
    
    if ( AgMD2::log(
             AgMD2_QueryMinWaveformMemory( md2.session(), 16, numRecords, 0, recordSize, &arraySize )
             , __FILE__, __LINE__ ) ) {

        auto mblk = std::make_shared< adportable::mblock<int16_t> >( arraySize );

		//std::unique_ptr< ViInt16[] > dataArray( new ViInt16[ arraySize ] );
        
        ViInt64 actualRecords(0), waveformArrayActualSize(0);
        std::vector<ViInt64> actualPoints( numRecords ), firstValidPoints( numRecords );
        std::vector<ViReal64> initialXOffset( numRecords ), initialXTimeSeconds( numRecords ), initialXTimeFraction( numRecords );
        ViReal64 xIncrement(0), scaleFactor(0), scaleOffset(0);

        auto tp = task::instance()->tp_acquire();
        
        if ( AgMD2::log( AgMD2_FetchMultiRecordWaveformInt16( md2.session()
                                                              , "Channel1"
                                                              , 0
                                                              , numRecords
                                                              , 0
                                                              , recordSize
                                                              , arraySize
                                                              , mblk->data()
                                                              , &waveformArrayActualSize
                                                              , &actualRecords
                                                              , actualPoints.data()
                                                              , firstValidPoints.data()
                                                              , initialXOffset.data()
                                                              , initialXTimeSeconds.data()
                                                              , initialXTimeFraction.data()
                                                              , &xIncrement
                                                              , &scaleFactor
                                                              , &scaleOffset ), __FILE__, __LINE__ ) ) {

            const auto& tp = task::instance()->tp_acquire();
            uint64_t acquire_tp_count = std::chrono::duration_cast<std::chrono::nanoseconds>( tp.time_since_epoch() ).count();

            for ( int64_t iRecord = 0; iRecord < actualRecords; ++iRecord ) {

                if ( auto data = std::make_shared< acqrscontrols::u5303a::waveform >( md2.Identify(), md2.dataSerialNumber() ) ) {
                    
                    auto& d = *data;
                    
                    d.method_ = m;
                    d.meta_.actualAverages = 0; // digitizer
                    d.meta_.actualPoints = actualPoints[ iRecord ];
                    d.meta_.initialXTimeSeconds = initialXTimeSeconds[ iRecord ] + initialXTimeFraction[ iRecord ];
                    d.meta_.xIncrement  = xIncrement;
                    d.meta_.initialXOffset = initialXOffset[ iRecord ];
                    d.meta_.scaleFactor = scaleFactor;
                    d.meta_.scaleOffset = scaleOffset;
                    d.meta_.dataType = mblk->dataType();

                    // time @ data read (U5303A has the large delay (~500us) after acquire trigger, so that read time is more accurate
                    d.timeSinceEpoch_ = std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::steady_clock::now().time_since_epoch() ).count();
                    d.setData( mblk, firstValidPoints[ iRecord ] );

                    vec.emplace_back( data );
                }
            }
            return true;
        }
    }
    return false;
}

bool
digitizer::readData16( AgMD2& md2, const acqrscontrols::u5303a::method& m, acqrscontrols::u5303a::waveform& data )
{
    const int64_t recordSize = m.method_.digitizer_nbr_of_s_to_acquire;
    ViInt64 const numRecords = 1;
    ViInt64 arraySize(0);

    if ( AgMD2::log(
             AgMD2_QueryMinWaveformMemory( md2.session(), 32, 1, 0, recordSize, &arraySize )
             , __FILE__, __LINE__ ) ) {

        ViInt32 actualAverages(0);
        ViInt64 actualRecords(0);
        ViInt64 actualPoints[numRecords] = {0}, firstValidPoint[numRecords] = {0};
        ViReal64 initialXTimeSeconds[numRecords] = {0}, initialXTimeFraction[numRecords] = {0};
        ViReal64 initialXOffset(0), xIncrement(0), scaleFactor(0), scaleOffset(0);

        const auto& tp = task::instance()->tp_acquire();        
        uint64_t acquire_tp_count = std::chrono::duration_cast<std::chrono::nanoseconds>( tp.time_since_epoch() ).count();
		auto mblk = std::make_shared< adportable::mblock<int32_t> >( arraySize );
        
        if ( AgMD2::log( AgMD2_FetchWaveformInt32( md2.session()
                                                   , "Channel1"
                                                   , arraySize
												   , reinterpret_cast<ViInt32*>( mblk->data() )
                                                   , actualPoints
                                                   , firstValidPoint
                                                   , &initialXOffset
                                                   , initialXTimeSeconds
                                                   , initialXTimeFraction
                                                   , &xIncrement
                                                   , &scaleFactor
                                                   , &scaleOffset ), __FILE__, __LINE__ ) ) {

            
            data.method_ = m;
            data.meta_.actualAverages = actualAverages;
            data.meta_.actualPoints = actualPoints[ 0 ];
            data.meta_.initialXTimeSeconds = initialXTimeSeconds[ 0 ] + initialXTimeFraction[ 0 ];
            data.meta_.xIncrement = xIncrement;
            data.meta_.initialXOffset  = initialXOffset;
            data.meta_.scaleFactor = scaleFactor;
            data.meta_.scaleOffset = scaleOffset;
            data.firstValidPoint_ = firstValidPoint[0];

            // time at data read (U5303A has the large delay (~500us) after acquire trigger, so that read time is more accurate
            data.timeSinceEpoch_ = std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::steady_clock::now().time_since_epoch() ).count();
            
            return true;
        }
    }
    return false;
}

bool
digitizer::readData32( AgMD2& md2, const acqrscontrols::u5303a::method& m, acqrscontrols::u5303a::waveform& data )
{
#if defined _MSC_VER
    ViInt64 const numRecords = 1;
#else
    ViInt64 constexpr numRecords = 1;
#endif
    const int64_t recordSize = m.method_.digitizer_nbr_of_s_to_acquire;
    ViInt64 arraySize(0);

    if ( AgMD2::log(
             AgMD2_QueryMinWaveformMemory( md2.session(), 32, 1, 0, recordSize, &arraySize )
             , __FILE__, __LINE__ ) ) {

        ViInt32 actualAverages(0);
        ViInt64 actualRecords(0);
        ViInt64 actualPoints[numRecords] = {0}, firstValidPoint[numRecords] = {0};
        ViReal64 initialXTimeSeconds[numRecords] = {0}, initialXTimeFraction[numRecords] = {0};
        ViReal64 initialXOffset(0), xIncrement(0), scaleFactor(0), scaleOffset(0);
        ViInt32 flags[numRecords];
        auto mblk = std::make_shared< adportable::mblock<int32_t> >( arraySize );

        const auto& tp = task::instance()->tp_acquire();
        uint64_t acquire_tp_count = std::chrono::duration_cast<std::chrono::nanoseconds>( tp.time_since_epoch() ).count();
        
        if ( AgMD2::log( AgMD2_FetchAccumulatedWaveformInt32( md2.session()
                                                              , "Channel1"
                                                              , 0
                                                              , 1
                                                              , 0
                                                              , recordSize
                                                              , arraySize
                                                              , reinterpret_cast<ViInt32*>( mblk->data() )
                                                              , &actualAverages
                                                              , &actualRecords
                                                              , actualPoints
                                                              , firstValidPoint
                                                              , &initialXOffset
                                                              , initialXTimeSeconds
                                                              , initialXTimeFraction
                                                              , &xIncrement
                                                              , &scaleFactor
                                                              , &scaleOffset, flags )
                         , __FILE__, __LINE__, [](){ return "FetchAccumulatedWaveformInt32()"; } ) ) {

            data.method_ = m;
            data.meta_.actualAverages = actualAverages;
            data.meta_.actualPoints = actualPoints[ 0 ];
            data.meta_.initialXTimeSeconds = initialXTimeSeconds[ 0 ] + initialXTimeFraction[ 0 ];
            data.meta_.xIncrement = xIncrement;
            data.meta_.initialXOffset = initialXOffset;
            data.meta_.scaleFactor = scaleFactor;
            data.meta_.scaleOffset = scaleOffset;
            data.timeSinceEpoch_ = acquire_tp_count + uint64_t( data.meta_.initialXTimeSeconds * 1.0e9 + 0.5 );
            data.firstValidPoint_ = firstValidPoint[0];
            data.setData( mblk, firstValidPoint[0] );
            
            return true;
        }
    }
    return false;
}
