/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adacquire/constants.hpp>
#include <adportable/date_string.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/mblock.hpp>
#include <adportable/profile.hpp>
#include <adportable/serializer.hpp>
#include <adportable/string.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <libdgpio/pio.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/type_traits.hpp>
#include <boost/variant.hpp>
#include <boost/json.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <exception>
#include <filesystem>
#include <mutex>
#include <ratio>
#include <thread>

namespace u5303a {

    namespace detail {

        class task : public fsm::handler {
            task();
            ~task();
        public:
            static task * instance();
            static const std::chrono::system_clock::time_point uptime_;
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
            bool dark( size_t );

            void connect( digitizer::command_reply_type f );
            void disconnect( digitizer::command_reply_type f );
            void connect( digitizer::waveform_reply_type f );
            void disconnect( digitizer::waveform_reply_type f );
            void setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw >& ptr );

            inline AgMD2 * spDriver() { return spDriver_.get(); }

            inline const acqrscontrols::u5303a::method& method() const { return method_; }

            inline const acqrscontrols::u5303a::identify& ident() const { return *ident_; }

            inline std::shared_ptr< acqrscontrols::u5303a::identify > ident_ptr() { return ident_; }

            inline bool isSimulated() const { return simulated_; }

            inline const std::chrono::system_clock::time_point& tp_acquire() const { return tp_acquire_; }

            void error_reply( const std::string& emsg, const std::string& );

            [[deprecated("replace with dgmod 2.0.4 hardwired")]] bool next_protocol( uint32_t protoIdx, uint32_t nProtocols );

        private:
            // fsm::handler
            void fsm_action_prepare() override;
            void fsm_action_stop() override;
            void fsm_action_TSR_stop() override;
            void fsm_action_initiate() override;
            void fsm_action_TSR_initiate() override;
            void fsm_action_continue() override;
            void fsm_action_TSR_continue() override;
            void fsm_action_halt() override;
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
            boost::asio::steady_timer timer_;
            bool simulated_;
            std::unique_ptr< dgpio::pio > pio_;
            acqrscontrols::u5303a::method method_;
            std::atomic_flag acquire_posted_;   // only one 'acquire' handler can be in the strand

            bool c_injection_requested_;
            bool c_acquisition_status_; // true: acq. is active, indeterminant: inj. waiting,
            size_t darkCount_;

            std::atomic<double> u5303_inject_timepoint_;
            std::vector< digitizer::command_reply_type > reply_handlers_;
            std::vector< digitizer::waveform_reply_type > waveform_handlers_;
            std::shared_ptr< acqrscontrols::u5303a::identify > ident_;
            std::shared_ptr< adportable::TimeSquaredScanLaw > scanlaw_;
            std::chrono::system_clock::time_point tp_acquire_;
            double temperature_;
            std::array< double, 2 > channel_temperature_;
            std::shared_ptr< const acqrscontrols::u5303a::waveform > darkWaveform_;

            bool handle_initial_setup();
            bool handle_terminating();
            bool handle_temperature();
            bool handle_acquire();
            bool handle_TSR_acquire();
            bool handle_prepare_for_run( const acqrscontrols::u5303a::method );
            bool handle_protocol( const acqrscontrols::u5303a::method );
            bool handle_timer( const boost::system::error_code& );
            bool acquire();
            bool waitForEndOfAcquisition( int timeout );
            bool readData( acqrscontrols::u5303a::waveform& );

            // PKD+AVG read Channel1+Channel2
            bool readDataPkdAvg( acqrscontrols::u5303a::waveform&, acqrscontrols::u5303a::waveform& );

            void set_time_since_inject( acqrscontrols::u5303a::waveform& );
        };

        struct device {
            static bool initial_setup( task&, const acqrscontrols::u5303a::method&, const std::string& options );
            static bool setup( task&, const acqrscontrols::u5303a::method& );
            static bool acquire( task& );
            static bool waitForEndOfAcquisition( task&, int timeout );
        };

        const std::chrono::system_clock::time_point task::uptime_ = std::chrono::system_clock::now();
        const uint64_t task::tp0_ = std::chrono::duration_cast<std::chrono::nanoseconds>( task::uptime_.time_since_epoch() ).count();
        /////
        struct configFile {
            static void saveResource( const std::string& res );
            static boost::optional< std::string > loadResource();
        };
        /////
    }
}

namespace {

}

using namespace u5303a;
using namespace u5303a::detail;

std::mutex task::mutex_;

digitizer::digitizer()
{
    boost::interprocess::shared_memory_object::remove( "waveform_simulator" );
#if WIN32
    _putenv_s( "AGMD2_SKIP_CAL_REQUIRED_CHECKS", "1" );
#else
    setenv( "AGMD2_SKIP_CAL_REQUIRED_CHECKS", "1", true );
#endif
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

bool
digitizer::peripheral_dark( size_t waitCount )
{
    return task::instance()->dark( waitCount );
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

task::task() : exptr_( nullptr )
             , digitizerNumRecords_( 1 )
             , fsm_( this )
             , work_( io_service_ )
             , strand_( io_service_ )
             , timer_( io_service_ )
             , simulated_( false )
             , pio_( std::make_unique< dgpio::pio >() )
             , c_injection_requested_( false )
             , c_acquisition_status_( false )
             , darkCount_( 0 )
             , u5303_inject_timepoint_( 0 )
             , temperature_( 0 )
             , channel_temperature_{{ 0 }}
{
    acquire_posted_.clear();

    pio_->open();

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
#if !defined NDEBUG && 0
    ADDEBUG() << "####################### task::initialize #########################";
#endif
	io_service_.post( strand_.wrap( [] { findResource(); } ) );

    io_service_.post( strand_.wrap( [this] { handle_initial_setup(); } ) );

    return true;
}

bool
task::next_protocol( uint32_t protoIdx, uint32_t nProtocols )
{
    // this method should be called same marshaling on readData(), which is under layer function of waveform_handler functor
    if ( protoIdx < method_.protocols().size() ) {

        if ( method_.setProtocolIndex( protoIdx, true ) ) {
            // don't call 'handle_protocol' from here.  It is for previous version of 'InfiTOF' plugin
            // next_protocol() method will be called from new implementation 'InfiTOF2' plugin
            device::setup( *this, method_ );
        }

        if ( simulated_ )
            simulator::instance()->setup( method_ );
    }
    return true;
}

bool
task::prepare_for_run( const acqrscontrols::u5303a::method& method )
{
#if !defined NDEBUG && 0
    auto& m = method._device_method();

    ADDEBUG() << "u5303a::task::prepare_for_run";
    ADDEBUG() << "\tfront_end_range: " << m.front_end_range << "\tfrontend_offset: " << m.front_end_offset
              << "\text_trigger_level: " << m.ext_trigger_level
              << "\tsamp_rate: " << m.samp_rate
              << "\tnbr_of_samples: " << m.nbr_of_s_to_acquire_ << "; " << m.digitizer_nbr_of_s_to_acquire
              << "\tnbr_of_average: " << m.nbr_of_averages
              << "\tdelay_to_first_s: " << adcontrols::metric::scale_to_micro( m.digitizer_delay_to_first_sample )
              << "\tinvert_signal: " << m.invert_signal;
    // << "\tnsa: " << m.nsa;

    ADDEBUG() << "##### u5303a::task::prepare_for_run - protocol size: " << method.protocols().size();
#endif

    io_service_.post( strand_.wrap( [=,this] { handle_prepare_for_run( method ); } ) );

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
    return fsm_.process_event( fsm::Stop() ) == boost::msm::back::HANDLED_TRUE;
}

bool
task::trigger_inject_out()
{
#if !defined NDEBUG && 0
    ADDEBUG() << "##### task::" << __FUNCTION__ << " #####";
#endif
    c_injection_requested_ = true;
    return true;
}

bool
task::dark( size_t waitCount )
{
    if ( method_.mode() == acqrscontrols::u5303a::method::DigiMode::Digitizer )
        return false;
    darkWaveform_.reset();
    darkCount_ = waitCount;
    for ( auto& reply: reply_handlers_ ) reply( waitCount ? "DarkStarted" : "DarkCanceled", "" );
    return waitCount;
}

void
task::terminate()
{
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
task::fsm_action_halt()
{
}

void
task::fsm_action_stop()
{
    // Non TSR stop
}

void
task::fsm_action_TSR_stop()
{
    spDriver_->abort();
}

void
task::fsm_action_prepare()
{

    if ( spDriver_->TSREnabled() )
        spDriver_->abort();
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
        waveform.wellKnownEvents_ |= adacquire::SignalObserver::wkEvent_INJECT;
#if !defined NDEBUG && 0
        ADDEBUG() << "### INJECTION on U5303A ## waveform.wellKnownEvents: " << waveform.wellKnownEvents_;
#endif
    }

    waveform.timeSinceInject_ = waveform.meta_.initialXTimeSeconds - u5303_inject_timepoint_;
    if ( c_acquisition_status_ )
        waveform.wellKnownEvents_ |= adacquire::SignalObserver::wkEvent_AcqInProgress;
}

bool
task::handle_initial_setup()
{
    spDriver_ = std::make_shared< AgMD2 >();     // spDriver creation in the thread

    bool simulated = false;
    bool success = false;

    const char * strInitOptions = "Simulate=false, DriverSetup= Model=U5303A";

    if ( auto p = getenv( "AcqirisOption" ) ) {
        if ( p && std::strcmp( p, "simulate" ) == 0 ) {
            strInitOptions = "Simulate=true, DriverSetup= Model=U5303A";
            auto code = spDriver_->initWithOptions( "PXI40::0::0::INSTR", VI_FALSE, VI_TRUE, strInitOptions );
            simulated = code == VI_SUCCESS;
            success = code == VI_SUCCESS;
            AgMD2::log( code, __FILE__,__LINE__ );
            ADDEBUG() << "##### U5303A SIMULATION MODE #####: " << strInitOptions << " code: " << code;
            if ( !success ) {
                ADDEBUG() << "############## SIMULATION MODE FAILED ################## " << boost::format("%x") % code;
                simulated = true;
            }
        }
    }

    if ( !simulated ) {
        std::string res;
        std::tie( success, res ) = u5303a::findResource()( spDriver_ );
        if ( success )
            ADTRACE() << "Initialize resource: " << res;
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

        if ( ! spDriver_->abort() )
            ADDEBUG() << "agmd2 abort failed";
        fsm_.process_event( fsm::Stop() );
    } else {
        fsm_.process_event( fsm::Error() );
    }

    for ( auto& reply: reply_handlers_ )
        reply( "InitialSetup", ( success ? "success" : "failed" ) );

    using namespace std::chrono_literals;

    timer_.expires_from_now( 6s );
    timer_.async_wait( [&]( const boost::system::error_code& ec ){ handle_timer(ec); } );

	return success;
}

bool
task::handle_temperature()
{
    AgMD2::log( AgMD2_QueryBoardTemperature( spDriver()->session(), &temperature_ ), __FILE__,__LINE__ );
    AgMD2::log( AgMD2_QueryChannelTemperature ( spDriver()->session(), "Channel1", &channel_temperature_[ 0 ] ),__FILE__,__LINE__ );

    bool interleaving(false);
    if ( ident_->Options().find( "INT" ) != std::string::npos ) {
        interleaving = true;
        AgMD2::log( AgMD2_QueryChannelTemperature ( spDriver()->session(), "Channel2", &channel_temperature_[ 1 ] ),__FILE__,__LINE__ );
    }

    auto jv =
        boost::json::object{{ "U5303A", {
                { "Temp", temperature_ }
                , { "CH1", channel_temperature_[ 0 ] }
                , { "CH2", channel_temperature_[ 1 ] }
                , { "ID",  ident_->SerialNumber() }
                , { "TP",   adportable::date_string::logformat( std::chrono::system_clock::now() ) }
            }
        }
    };

    for ( auto& reply: reply_handlers_ ) {
        const std::string method = isSimulated() ? "Temperature.simulated" : "Temperature";
        reply( method, boost::json::serialize(jv) );
    }

    return true;
}

bool
task::handle_timer( const boost::system::error_code& ec )
{
    using namespace std::chrono_literals;

    if ( ec != boost::asio::error::operation_aborted ) {

        strand_.post( [&] { handle_temperature(); } );

        boost::system::error_code erc;
        timer_.expires_from_now( 6s, erc );
        if ( !erc )
            timer_.async_wait( [&]( const boost::system::error_code& ec ){ handle_timer( ec ); });

    }
    return true;
}

bool
task::handle_terminating()
{
	return false;
}

bool
task::handle_prepare_for_run( const acqrscontrols::u5303a::method m )
{
    if ( fsm_.process_event( fsm::Prepare() ) == boost::msm::back::HANDLED_FALSE )
        return false;

    c_acquisition_status_ = false;
    c_injection_requested_ = false;
    u5303_inject_timepoint_ = 0;

    device::initial_setup( *this, m, ident().Options() );

    if ( /* m.mode_ && */ simulated_ ) {
        acqrscontrols::u5303a::method a( m );
        a._device_method().samp_rate = spDriver()->SampleRate();
        simulator::instance()->setup( a );
    }

    method_ = m;

    if ( m._device_method().TSR_enabled ) {
        return fsm_.process_event( fsm::TSRInitiate() ) == boost::msm::back::HANDLED_TRUE;
    } else {
        return fsm_.process_event( fsm::Initiate() ) == boost::msm::back::HANDLED_TRUE;
    }

}

bool
task::handle_protocol( const acqrscontrols::u5303a::method m )
{
    device::setup( *this, m );

    if ( simulated_ )
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

    auto tp = std::chrono::system_clock::now() + std::chrono::milliseconds( 1000 ); // wait for max 1 second

    boost::tribool complete;
    while ( ! ( complete = spDriver()->isTSRAcquisitionComplete() ) && ( std::chrono::system_clock::now() < tp ) ) {
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

        using acqrscontrols::u5303a::method;
        if ( waitForEndOfAcquisition( 3000 ) ) {
            if ( method_.mode() == method::DigiMode::Digitizer ) { // digitizer

                int protocolIndex = pio_->protocol_number(); // <- hard wired protocol id

                if ( protocolIndex < 0 && simulated_ )
                	protocolIndex = simulator::instance()->protocol_number();

                if ( protocolIndex >= 0 )
                    method_.setProtocolIndex( protocolIndex & 03, false ); // 2bits

                std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > > vec;

                // read multiple waveform using mblock
                digitizer::readData( *spDriver(), method_, vec );

                if ( simulated_ )
                    simulator::instance()->touchup( vec, method_ );

                for ( auto& waveform: vec ) {
                    set_time_since_inject( *waveform );         // <---------- INJECTION event set ------------
                    acqrscontrols::u5303a::method m;
                    for ( auto& reply: waveform_handlers_ ) {
                        if ( reply( waveform.get(), nullptr, m ) )
                            handle_protocol( m );
                    }
                }

            } else { // average
                uint32_t events = darkCount_ ? adacquire::SignalObserver::wkEvent_DarkInProgress : 0;

                if ( method_._device_method().pkd_enabled ) {
                    // PKD+AVG
                    auto pkd = std::make_shared< acqrscontrols::u5303a::waveform >( ident_, events );
                    auto avg = std::make_shared< acqrscontrols::u5303a::waveform >( ident_, events );
                    if ( readDataPkdAvg( *pkd, *avg ) ) {
                        set_time_since_inject( *pkd );          // <---------- INJECTION event set ------------
                        set_time_since_inject( *avg );          // <---------- INJECTION event set ------------
                        acqrscontrols::u5303a::method m;
                        for ( auto& reply : waveform_handlers_ ) {
                            if ( reply( avg.get(), pkd.get(), m ) )
                                handle_protocol( m );
                        }
                        auto dark( darkWaveform_ );
                        if ( darkCount_ && --darkCount_ == 0 ) {
                            darkWaveform_ = avg;
                            for ( auto& reply: reply_handlers_ ) reply( "DarkAcquired", "" );
                        }
                        if ( dark )
                            avg->darkSubtraction( *dark );
                    }

                } else {
                    // AVERAGE
                    auto waveform = std::make_shared< acqrscontrols::u5303a::waveform >( ident_, events );
                    if ( readData( *waveform ) ) {
                        set_time_since_inject( *waveform );     // <---------- INJECTION event set ------------
                        acqrscontrols::u5303a::method m;
                        for ( auto& reply : waveform_handlers_ ) {
                            if ( reply( waveform.get(), nullptr, m ) )
                                handle_protocol( m );
                        }
                        auto dark( darkWaveform_ );
                        if ( darkCount_ && --darkCount_ == 0 ) {
                            darkWaveform_ = waveform;
                            for ( auto& reply: reply_handlers_ ) reply( "DarkAcquired", "" );
                        }
                        if ( dark )
                            waveform->darkSubtraction( *dark );
                    }
                }
            }
            return true;
        } else {
            ADDEBUG() << "Acquisition timed out";
        }
    } else {
        std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
        if ( ! spDriver_->isIdle() )
            spDriver_->abort();
    }
    return false;
}

bool
task::acquire()
{
    if ( (method_.mode() == acqrscontrols::u5303a::method::DigiMode::Averager) && simulated_ )
        return simulator::instance()->acquire();

    tp_acquire_ = std::chrono::system_clock::now();
    return device::acquire( *this );
}

bool
task::waitForEndOfAcquisition( int timeout )
{
    if ( simulated_ ) {
        using namespace std::chrono_literals;
#if defined _MSC_VER && defined _DEBUG
        std::this_thread::sleep_for( 100ms );
#else
        std::this_thread::sleep_for( 10ms );
#endif
        if ( method_.mode() == acqrscontrols::u5303a::method::DigiMode::Averager )
            return simulator::instance()->waitForEndOfAcquisition();
    }

    return device::waitForEndOfAcquisition( *this, timeout );
}

bool
task::readDataPkdAvg( acqrscontrols::u5303a::waveform& pkd, acqrscontrols::u5303a::waveform& avg )
{
    pkd.serialnumber_ = spDriver()->dataSerialNumber();
    avg.serialnumber_ = spDriver()->dataSerialNumber();
    set_time_since_inject( pkd );
    set_time_since_inject( avg );

    if ( simulated_ ) {
        simulator::instance()->readDataPkdAvg( pkd, avg );
        //  pkd.timeSinceEpoch_ =  std::chrono::system_clock::now().time_since_epoch().count(); // <-- should be recorded in nanoseconds, 2024/03/13
        // pkd.timeSinceEpoch_ = std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds >::now().time_since_epock().count();
        pkd.timeSinceEpoch_ = std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::system_clock::now().time_since_epoch()).count();
        avg.timeSinceEpoch_ = pkd.timeSinceEpoch_;
        pkd.meta_.channelMode = acqrscontrols::u5303a::PKD;
        avg.meta_.channelMode = acqrscontrols::u5303a::AVG;
        return true;
    }

    digitizer::readData32( *spDriver(), method_, pkd, "Channel1" );
    pkd.meta_.channelMode = acqrscontrols::u5303a::PKD;

    digitizer::readData32( *spDriver(), method_, avg, "Channel2" );
    avg.meta_.channelMode = acqrscontrols::u5303a::AVG;

    return true;
}

bool
task::readData( acqrscontrols::u5303a::waveform& data )
{
    data.serialnumber_ = spDriver()->dataSerialNumber();

    if ( simulated_ ) {
        if ( simulator::instance()->readData( data ) ) {
            data.timeSinceEpoch_ = std::chrono::system_clock::now().time_since_epoch().count();
            set_time_since_inject( data ); // ==> set elapsed time for debugging
            return true;
        }
    }
    //-------------------

    bool result( false );
    if ( method_.mode() == acqrscontrols::u5303a::method::DigiMode::Digitizer ) {
        result = digitizer::readData16( *spDriver(), method_, data );
    } else {
        result = digitizer::readData32( *spDriver(), method_, data, "Channel1" );
        data.meta_.channelMode = acqrscontrols::u5303a::AVG;
    }
    set_time_since_inject( data ); // ==> set elapsed time for debugging
    return result;
}

void
task::connect( digitizer::command_reply_type f )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    reply_handlers_.emplace_back( f );
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
    waveform_handlers_.emplace_back( f );
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
    constexpr std::array< std::pair< const char *, double >, 4 >
        input_specs = { {{"SR0", 0.5e9}, {"SR1", 1.0e9}, {"SR2", 1.6e9}, {"SR3", 2.0e9 }} };

    auto it = std::find_if( input_specs.begin(), input_specs.end(), [&]( auto& a ) { return options.find( a.first ) != options.npos; } );
    if ( it == input_specs.end() ) // no input specification found
        return false;

    const double input_rate = it->second;

    bool interleave = ( options.find("INT") != options.npos ) && ( m._device_method().samp_rate > input_rate );
    const bool pkd_enabled = m._device_method().pkd_enabled && ( options.find( "PKD" ) != options.npos );

    ADDEBUG() << "device_method.pkd_enabled: " << m._device_method().pkd_enabled << ", " << ( options.find( "PKD" ) != options.npos );

    if ( m._device_method().pkd_enabled && ( options.find( "PKD" ) == options.npos ) )
        adlog::logger(__FILE__,__LINE__,adlog::LOG_WARNING) << "U5303A does not support requested function 'PKD'";

    if ( pkd_enabled )
        interleave = false;  // force disable interleaving

    double max_rate = interleave ? input_rate * 2 : input_rate;
//#if !defined NDEBUG && 0
    ADINFO() << "##### Supported max. sample rate: " << max_rate << "\tChannel rate: " << input_rate;
    ADINFO() << "##### User specified sample rate: " << m._device_method().samp_rate << (interleave ? " w/ interleave" : " w/o interleave");
//#endif
    if ( interleave ) {
        task.spDriver()->ConfigureTimeInterleavedChannelList( "Channel1", "Channel2" );
    } else {
        task.spDriver()->ConfigureTimeInterleavedChannelList( "Channel1", "" );
    }

    AgMD2::log( AgMD2_ConfigureChannel( task.spDriver()->session(), "Channel1"
                                        , m._device_method().front_end_range
                                        , m._device_method().front_end_offset, AGMD2_VAL_VERTICAL_COUPLING_DC, VI_TRUE ), __FILE__, __LINE__ );

    attribute< active_trigger_source >::set( *task.spDriver(), std::string( "External1" ) );
    attribute< trigger_level >::set( *task.spDriver(), "External1", m._device_method().ext_trigger_level );
    attribute< trigger_slope >::set( *task.spDriver(), "External1", AGMD2_VAL_POSITIVE );
    attribute< trigger_coupling >::set( *task.spDriver(), "External1", AGMD2_VAL_TRIGGER_COUPLING_DC );
    attribute< trigger_delay >::set( *task.spDriver(), m._device_method().delay_to_first_sample_ );

    //bool success = false;

    const double samp_rate = m._device_method().samp_rate > max_rate ? max_rate : m._device_method().samp_rate;

    if ( ! AgMD2::log( attribute< u5303a::sample_rate >::set( *task.spDriver(), samp_rate ), __FILE__,__LINE__ ) ) {
        AgMD2::log( attribute< u5303a::sample_rate >::set( *task.spDriver(), max_rate ), __FILE__,__LINE__ );
    }

    if ( m.mode() == acqrscontrols::u5303a::method::DigiMode::Digitizer ) { // Digitizer
#if !defined NDEBUG && 0
        ADINFO() << "##### --> digitizer mode";
#endif
        task.spDriver()->setTSREnabled( m._device_method().TSR_enabled );
        task.spDriver()->setAcquisitionMode( AGMD2_VAL_ACQUISITION_MODE_NORMAL );
        task.spDriver()->setAcquisitionRecordSize( m._device_method().nbr_of_s_to_acquire_ );
        task.spDriver()->setAcquisitionNumRecordsToAcquire( m._device_method().nbr_records );

    } else { // Averager

        // ADDEBUG() << "Averager Mode";
        task.spDriver()->setTSREnabled( false );

        // PKD - POC
        if ( m._device_method().pkd_enabled && options.find( "PKD" ) != options.npos ) {

            ADDEBUG() << "================================== PKD ==========================================";
            ADINFO() << "##### PKD ON; Invert signal " << ( m._device_method().invert_signal ? "true" : "false" )
                     << "; Amplitude accum. " << (m._device_method().pkd_amplitude_accumulation_enabled ? "enabled" : "disabled");

            AgMD2::log( attribute< num_records_to_acquire >::set( *task.spDriver(), int64_t( 1 ) ), __FILE__,__LINE__ );
            AgMD2::log( attribute< acquisition_mode >::set( *task.spDriver(), AGMD2_VAL_ACQUISITION_MODE_PEAK_DETECTION ), __FILE__,__LINE__ );

            // Configure the data inversion mode - VI_FALSE (no data inversion) by default
            AgMD2::log( attribute< channel_data_inversion_enabled >::set( *task.spDriver()
                                                                          , "Channel1"
                                                                          , bool( m._device_method().invert_signal ) ), __FILE__,__LINE__ );

            // Configure the accumulation enable mode: the peak value is stored (VI_TRUE) or the peak value is forced to '1' (VI_FALSE).
            AgMD2::log( attribute< peak_detection_amplitude_accumulation_enabled >::set(
                            *task.spDriver(), "Channel1", m._device_method().pkd_amplitude_accumulation_enabled )
                        , __FILE__,__LINE__ );

            // Configure the RisingDelta and FallingDelta in LSB: define the amount by which two consecutive samples must differ to be
            // considered as rising/falling edge in the peak detection algorithm.

            AgMD2::log( attribute< peak_detection_rising_delta >::set( *task.spDriver(), "Channel1", m._device_method().pkd_raising_delta ), __FILE__,__LINE__ );
            AgMD2::log( attribute< peak_detection_falling_delta >::set( *task.spDriver(), "Channel1", m._device_method().pkd_falling_delta ), __FILE__,__LINE__ );

            AgMD2::log( attribute< record_size >::set( *task.spDriver(), m._device_method().nbr_of_s_to_acquire_ ), __FILE__,__LINE__ );

            AgMD2::log( attribute< num_records_to_acquire >::set( *task.spDriver(), int64_t( 1 ) ), __FILE__,__LINE__ );

            AgMD2::log( attribute< acquisition_number_of_averages >::set( *task.spDriver(), m._device_method().nbr_of_averages ), __FILE__,__LINE__ );

        } else {
            // ADINFO() << "##### AVG ON; Invert signal " << ( m._device_method().invert_signal ? "true" : "false" );

            AgMD2::log( attribute< num_records_to_acquire >::set( *task.spDriver(), int64_t( 1 ) ), __FILE__,__LINE__ );

            AgMD2::log( attribute< acquisition_mode >::set( *task.spDriver(), AGMD2_VAL_ACQUISITION_MODE_AVERAGER ), __FILE__,__LINE__ );

            AgMD2::log( attribute< channel_data_inversion_enabled >::set( *task.spDriver()
                                                                          , "Channel1"
                                                                          , bool( m._device_method().invert_signal ) ), __FILE__,__LINE__ );

            AgMD2::log( attribute< record_size >::set( *task.spDriver(), m._device_method().nbr_of_s_to_acquire_ ), __FILE__,__LINE__ );

            //It looks like this command should be issued at last
            AgMD2::log( attribute< acquisition_number_of_averages >::set( *task.spDriver(), m._device_method().nbr_of_averages ), __FILE__,__LINE__ );
        }
    }

    // ADTRACE() << "##### ACQUISITION_MODE : " << task.spDriver()->AcquisitionMode();

    task.spDriver()->CalibrationSelfCalibrate();

	return true;
}

bool
device::setup( task& task, const acqrscontrols::u5303a::method& m )
{
    // Don't forget environment variable: 'AGMD2_SKIP_CAL_REQUIRED_CHECKS=1'

    task.spDriver()->setTriggerDelay( m._device_method().delay_to_first_sample_ );
    task.spDriver()->setAcquisitionRecordSize( m._device_method().nbr_of_s_to_acquire_ );
    task.spDriver()->setAcquisitionNumRecordsToAcquire( m._device_method().nbr_records );

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
    auto tp = std::chrono::system_clock::now() + std::chrono::milliseconds( timeout );

    while( ! task.spDriver()->isAcquisitionIdle() ) {
        if ( tp < std::chrono::system_clock::now() )
            return false;
    }
    return true;
}

///////////////
acqrscontrols::u5303a::method
digitizer::method() const
{
    return task::instance()->method();
}

/////////////
bool
digitizer::readData( AgMD2& md2, const acqrscontrols::u5303a::method& m, std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > >& vec )
{
    ViInt64 arraySize = 0;
    const int64_t recordSize = m._device_method().digitizer_nbr_of_s_to_acquire;
    const int64_t numRecords = m._device_method().nbr_records;

    if ( AgMD2::log(
             AgMD2_QueryMinWaveformMemory( md2.session(), 16, numRecords, 0, recordSize, &arraySize )
             , __FILE__, __LINE__ ) ) {

        auto mblk = std::make_shared< adportable::mblock<int16_t> >( arraySize );
        std::fill( mblk->data(), mblk->data() + arraySize, 0 );

        ViInt64 actualRecords(0), waveformArrayActualSize(0);
        std::vector<ViInt64> actualPoints( numRecords ), firstValidPoints( numRecords );
        std::vector<ViReal64> initialXOffset( numRecords ), initialXTimeSeconds( numRecords ), initialXTimeFraction( numRecords );
        ViReal64 xIncrement(0), scaleFactor(0), scaleOffset(0);

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

            // const auto& tp = task::instance()->tp_acquire();
            // uint64_t acquire_tp_count = std::chrono::duration_cast<std::chrono::nanoseconds>( tp.time_since_epoch() ).count();

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
                    d.meta_.protocolIndex = m.protocolIndex();
                    d.meta_.dataType = mblk->dataType();

                    d.timeSinceEpoch_ = std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::system_clock::now().time_since_epoch() ).count();
                    d.setData( mblk, firstValidPoints[ iRecord ] );

                    // ADDEBUG() << "readData: " << d.method_.protocolIndex() << "/" << d.method_.protocols().size()
                    //           << " SF=" << scaleFactor << " SO=" << scaleOffset << " t=" << d.meta_.dataType
                    //           << " data[10]=" << boost::format( "%x" ) % mblk->data()[ 10 ];

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
    const int64_t recordSize = m._device_method().digitizer_nbr_of_s_to_acquire;
    ViInt64 const numRecords = 1;
    ViInt64 arraySize(0);

    if ( AgMD2::log(
             AgMD2_QueryMinWaveformMemory( md2.session(), 32, 1, 0, recordSize, &arraySize )
             , __FILE__, __LINE__ ) ) {

        ViInt32 actualAverages(0);
        ViInt64 actualPoints[numRecords] = {0}, firstValidPoint[numRecords] = {0};
        ViReal64 initialXTimeSeconds[numRecords] = {0}, initialXTimeFraction[numRecords] = {0};
        ViReal64 initialXOffset(0), xIncrement(0), scaleFactor(0), scaleOffset(0);

        //const auto& tp = task::instance()->tp_acquire();
        //uint64_t acquire_tp_count = std::chrono::duration_cast<std::chrono::nanoseconds>( tp.time_since_epoch() ).count();

		auto mblk = std::make_shared< adportable::mblock<int16_t> >( arraySize );

        if ( AgMD2::log( AgMD2_FetchWaveformInt16( md2.session()
                                                   , "Channel1"
                                                   , arraySize
												   , reinterpret_cast<ViInt16*>( mblk->data() )
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
            data.meta_.protocolIndex = m.protocolIndex();
            data.meta_.dataType = 2;
            data.firstValidPoint_ = firstValidPoint[0];
            data.timeSinceEpoch_ = std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::system_clock::now().time_since_epoch() ).count();
            data.setData( mblk, firstValidPoint[0] );

            return true;
        }
    }
    return false;
}

bool
digitizer::readData32( AgMD2& md2, const acqrscontrols::u5303a::method& m, acqrscontrols::u5303a::waveform& data
                       , const char * channel )
{
    ViInt64 constexpr numRecords = 1;
    const int64_t recordSize = m._device_method().digitizer_nbr_of_s_to_acquire;
    ViInt64 arraySize(0);

    if ( AgMD2::log(
             AgMD2_QueryMinWaveformMemory( md2.session(), 32, numRecords, 0, recordSize, &arraySize )
             , __FILE__, __LINE__ ) ) {

        ViInt32 actualAverages(0);
        ViInt64 actualRecords(0);
        ViInt64 actualPoints[numRecords] = {0}, firstValidPoint[numRecords] = {0};
        ViReal64 initialXTimeSeconds[numRecords] = {0}, initialXTimeFraction[numRecords] = {0};
        ViReal64 initialXOffset(0), xIncrement(0), scaleFactor(0), scaleOffset(0);
        ViInt32 flags[numRecords];
        auto mblk = std::make_shared< adportable::mblock< int32_t > >( arraySize );

        // const auto& tp = task::instance()->tp_acquire();
        // uint64_t acquire_tp_count = std::chrono::duration_cast<std::chrono::nanoseconds>( tp.time_since_epoch() ).count();

        if ( AgMD2::log( AgMD2_FetchAccumulatedWaveformInt32( md2.session()
                                                              , channel // "Channel1"
                                                              , 0
                                                              , numRecords // 1
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
#if 0
            ADDEBUG() << channel;
            int32_t * pdata = reinterpret_cast<ViInt32*>( mblk->data() );
            for ( int i = 0; i < 4; ++i )
                ADDEBUG() << pdata[i];
#endif

            data.method_ = m;
            data.meta_.actualAverages = actualAverages;
            data.meta_.actualPoints = actualPoints[ 0 ];
            data.meta_.initialXTimeSeconds = initialXTimeSeconds[ 0 ] + initialXTimeFraction[ 0 ];
            data.meta_.xIncrement = xIncrement;
            data.meta_.initialXOffset = initialXOffset;
            data.meta_.scaleFactor = scaleFactor;
            data.meta_.scaleOffset = scaleOffset;
            data.meta_.protocolIndex = m.protocolIndex();
            data.meta_.dataType = 4;
            data.firstValidPoint_ = firstValidPoint[0];
            data.timeSinceEpoch_ = std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::system_clock::now().time_since_epoch() ).count();
            data.setData( mblk, firstValidPoint[0] );

            return true;
        }
    }
    return false;
}

std::pair< bool, std::string >
findResource::operator()( std::shared_ptr< AgMD2 > md2 ) const
{
    const char * strInitOptions = "Simulate=false, DriverSetup= Model=U5303A";

    if ( auto res = configFile::loadResource() ) {
        if ( md2->initWithOptions( res.get(), VI_FALSE, VI_TRUE, strInitOptions ) == VI_SUCCESS )
            return std::make_pair( true, res.get() );
    }
    for ( int num = 0; num < 199; num++ ) {
        std::string res = ( boost::format("PXI%d::0::0::INSTR") % num ).str();
        if ( md2->initWithOptions( res, VI_FALSE, VI_TRUE, strInitOptions ) == VI_SUCCESS ) {
            configFile::saveResource( res );
            return std::make_pair( true, res );
        }
        ADDEBUG() << "Resource: " << res << "\tfaild";
    }
    return std::make_pair( false, "" );
}

//static
void
configFile::saveResource( const std::string& res )
{
    const std::filesystem::path path = std::filesystem::path( adportable::profile::user_config_dir<char>() ) / "QtPlatz";
    const std::filesystem::path file = path / "digitizer.ini";

    if ( ! std::filesystem::exists( path ) ) {
        boost::system::error_code ec;
        std::filesystem::create_directories( path, ec );
        if ( ec ) {
            ADDEBUG() << ec;
            return; // error
        }
    }
    boost::property_tree::ptree pt;
    pt.put( "U5303A.resource", res );
    boost::property_tree::write_ini( file.string(), pt );
}

//static
boost::optional< std::string >
configFile::loadResource()
{
    const std::filesystem::path path = std::filesystem::path( adportable::profile::user_config_dir<char>() ) / "QtPlatz";
    const std::filesystem::path file = path / "digitizer.ini";

    if ( std::filesystem::exists( file ) ) {

        boost::property_tree::ptree pt;
        try {
            boost::property_tree::read_ini( file.string(), pt );
        } catch ( std::exception& ex ) {
            ADDEBUG() << ex.what();
        }
        return pt.get_optional<std::string>( "U5303A.resource" );
    }
    return boost::none;
}
