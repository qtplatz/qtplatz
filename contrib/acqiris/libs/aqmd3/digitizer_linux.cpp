/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "aqmd3.hpp"
#include "automaton.hpp"
#include "configfile.hpp"
#include <aqmd3/findresource.hpp>
#include <aqmd3controls/method.hpp>
#include <adacquire/constants.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adlog/logger.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/mblock.hpp>
#include <adportable/profile.hpp>
#include <adportable/serializer.hpp>
#include <adportable/string.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <libdgpio/pio.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/optional.hpp>
#include <boost/type_traits.hpp>
#include <boost/variant.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#ifndef NDEBUG
#include <boost/format.hpp>
#endif
#include <algorithm>
#include <atomic>
#include <chrono>
#include <exception>
#include <mutex>
#include <thread>

namespace aqmd3 {

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
            bool prepare_for_run( const aqmd3controls::method& );
            bool run();
            bool stop();
            bool trigger_inject_out();
            bool dark( size_t );

            void connect( digitizer::command_reply_type f );
            void disconnect( digitizer::command_reply_type f );
            void connect( digitizer::waveform_reply_type f );
            void disconnect( digitizer::waveform_reply_type f );
            void setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw >& ptr );

            inline AqMD3 * spDriver() { return spDriver_.get(); }
            inline std::shared_ptr< const aqmd3controls::method > method() const { return method_; }
            inline const aqmd3controls::identify& ident() const { return *ident_; }
            inline std::shared_ptr< aqmd3controls::identify > ident_ptr() { return ident_; }
            inline bool isSimulated() const { return simulated_; }
            inline const std::chrono::system_clock::time_point& tp_acquire() const { return tp_acquire_; }

            void error_reply( const std::string& emsg, const std::string& );

            // helper method
            ViStatus log( ViStatus rcode, const char * const file, int line
                          , std::function< std::string() > details = std::function< std::string() >() ) const;

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
            std::shared_ptr< AqMD3 > spDriver_;
            std::vector< adportable::asio::thread > threads_;
            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;
            boost::asio::io_service::strand strand_;
            boost::asio::steady_timer timer_;
            bool simulated_;
            std::unique_ptr< dgpio::pio > pio_;
            std::shared_ptr< aqmd3controls::method > method_;
            std::atomic_flag acquire_posted_;   // only one 'acquire' handler can be in the strand

            bool c_injection_requested_;
            bool c_acquisition_status_; // true: acq. is active, indeterminant: inj. waiting,
            size_t darkCount_;

            std::atomic<double> u5303_inject_timepoint_;
            // std::vector< std::string > listResources_;
            std::vector< digitizer::command_reply_type > reply_handlers_;
            std::vector< digitizer::waveform_reply_type > waveform_handlers_;
            std::shared_ptr< aqmd3controls::identify > ident_;
            std::shared_ptr< adportable::TimeSquaredScanLaw > scanlaw_;
            std::chrono::system_clock::time_point tp_acquire_;
            double temperature_;
            std::array< double, 2 > channel_temperature_;
            std::shared_ptr< const aqmd3controls::waveform > darkWaveform_;

            bool handle_initial_setup();
            bool handle_terminating();
            bool handle_temperature();
            bool handle_acquire();
            bool handle_TSR_acquire();
            bool handle_prepare_for_run( const aqmd3controls::method );
            bool handle_protocol( const aqmd3controls::method );
            bool handle_timer( const boost::system::error_code& );
            bool acquire();
            bool waitForEndOfAcquisition( int timeout );
            bool readData( aqmd3controls::waveform& );

            // PKD+AVG read Channel1+Channel2
            bool readDataPkdAvg( aqmd3controls::waveform&, aqmd3controls::waveform&, int64_t epoch_time );

            void set_time_since_inject( aqmd3controls::waveform& );
        };

        struct device {
            static bool validate( std::shared_ptr< aqmd3::AqMD3>, aqmd3controls::method& );
            static bool initial_setup( std::shared_ptr< aqmd3::AqMD3 >, const aqmd3controls::method&, const std::string& options );
            static bool setup( std::shared_ptr< aqmd3::AqMD3 >, const aqmd3controls::method& );
            static bool acquire( std::shared_ptr< aqmd3::AqMD3 > );
            static bool waitForEndOfAcquisition( std::shared_ptr< aqmd3::AqMD3 >, int timeout );
        };

        const std::chrono::system_clock::time_point task::uptime_ = std::chrono::system_clock::now();
        const uint64_t task::tp0_ = std::chrono::duration_cast<std::chrono::nanoseconds>( task::uptime_.time_since_epoch() ).count();

        /////
        struct waveform_print {
            std::chrono::steady_clock::time_point tp_;
            template< typename T >
            void operator()( const aqmd3controls::waveform& data, const T * mblk ) {
                using namespace std::chrono_literals;
                using namespace aqmd3controls;
                auto tp = std::chrono::steady_clock::now();
                if ( ( tp - tp_ ) > 1s ) {
                    tp_ = tp;
                    ADDEBUG() << std::make_pair( data.xmeta().scaleFactor, data.xmeta().scaleOffset );
                    for ( int i = 0; i < 8; ++i )
                        ADDEBUG() << mblk[i] << ", " << data.data()[i]
                                  << boost::format( ",\tA: %.4f" ) % waveform::toVolts_< int32_t
                                                                                         , method::DigiMode::Averager >()( data.xmeta(), data.data()[i] )
                                  << boost::format( ",\tD: %.4f" ) % waveform::toVolts_< int32_t
                                                                                         , method::DigiMode::Digitizer >()( data.xmeta(), data.data()[i] )
                            ;
                }
            }
        };

        const static std::vector< std::string > ModelSA = { "SA217E", "SA217P", "SA220P", "SA220E", "SA230P" "SA230E" };

    }

}

namespace {
    struct acqirisOption {
        boost::optional< std::string > operator()() const {
            if ( auto p = getenv( "AcqirisOption" ) ) {
                auto env = std::string( p );
                if ( auto simulate = boost::algorithm::contains( env, "simulate" ) ) {
                    std::vector< std::string > options = {{ "Simulate=true" }, {"DriverSetup="}};
                    if ( auto it = boost::algorithm::find_first( env, "Model=" ) ) {

                    }
                }
            }
            return {};
        }
    };
}

using namespace aqmd3;
using namespace aqmd3::detail;

std::mutex task::mutex_;

digitizer::digitizer()
{
    boost::interprocess::shared_memory_object::remove( "waveform_simulator" );
#if WIN32
    _putenv_s( "AQMD3_SKIP_CAL_REQUIRED_CHECKS", "1" );
#else
    setenv( "AQMD3_SKIP_CAL_REQUIRED_CHECKS", "1", true );
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

    ADDEBUG() << __FUNCTION__;

    adcontrols::ControlMethod::Method cm( m );
    cm.sort();

#ifndef NDEBUG
    ADDEBUG() << "###################################################################";
    for ( auto mi: cm ) {
        ADDEBUG() << __FUNCTION__ << "------------- modelname: " << mi.modelname();
    }
#endif

    auto it = std::find_if( cm.begin(), cm.end(), [] ( const MethodItem& mi ){ return mi.modelname() == "u5303a"; } );
    if ( it != cm.end() ) {
        ADDEBUG() << __FUNCTION__ << " -- u5303a method found.";
        aqmd3controls::method m;

        if ( it->get( *it, m ) )
            return task::instance()->prepare_for_run( m );
    } else {
        ADDEBUG() << __FUNCTION__ << " -- no u5303a method found. <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
    }

    return false;
}

bool
digitizer::peripheral_prepare_for_run( const aqmd3controls::method& m )
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
#ifndef NDEBUG
    ADDEBUG() << "##### " << __FUNCTION__ << " #####";
#endif
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
             , method_( std::make_shared< aqmd3controls::method >() )
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

    threads_.emplace_back( adportable::asio::thread( [this]() {
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
#if !defined NDEBUG
    ADDEBUG() << "=================== task::initialize =================";
#endif
	//io_service_.post( strand_.wrap( [this] { findResource(); } ) );
    io_service_.post( strand_.wrap( [this] { handle_initial_setup(); } ) );

    return true;
}

bool
task::prepare_for_run( const aqmd3controls::method& method )
{
#if !defined NDEBUG
    auto& m = method.device_method();

    ADDEBUG() << "aqmd3::digitizer_linux::task::prepare_for_run"
              << "\n\tfront_end_range: " << m.front_end_range << "\tfrontend_offset: " << m.front_end_offset
              << "\n\text_trigger_level: " << m.ext_trigger_level
              << "\n\tsamp_rate: " << m.samp_rate
              << "\n\tnbr_of_samples: " << m.nbr_of_s_to_acquire_ << "; " << m.digitizer_nbr_of_s_to_acquire
              << "\n\tnbr_of_average: " << m.nbr_of_averages
              << "\n\tdelay_to_first_s: " << adcontrols::metric::scale_to_micro( m.digitizer_delay_to_first_sample )
              << "\n\tinvert_signal: " << m.invert_signal;
        // << "\tnsa: " << m.nsa;

    ADDEBUG() << "##### aqmd3::digitizer_linux::task::prepare_for_run - protocol size: " << method.protocols().size();
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
    return fsm_.process_event( fsm::Stop() ) == boost::msm::back::HANDLED_TRUE;
}

bool
task::trigger_inject_out()
{
#ifndef NDEBUG
    ADDEBUG() << "##### task::" << __FUNCTION__ << " #####";
#endif
    c_injection_requested_ = true;
    return true;
}

bool
task::dark( size_t waitCount )
{
    if ( method_->mode() == aqmd3controls::method::DigiMode::Digitizer )
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
    if ( ident_->Options().find( "TSR" ) != std::string::npos ) {
        ViStatus rc;
        if ( auto enabled = attribute< tsr_enabled >::value( *spDriver_, rc ) )
            if ( enabled.get() )
                spDriver_->abort();
    }
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
task::set_time_since_inject( aqmd3controls::waveform& waveform )
{
    if ( c_injection_requested_ ) {
        c_injection_requested_ = false;
        c_acquisition_status_ = true;
        u5303_inject_timepoint_ = waveform.xmeta().initialXTimeSeconds;
        waveform.set_well_known_events( waveform.well_known_events() | adacquire::SignalObserver::wkEvent_INJECT );

        ADDEBUG() << "## INJECTION on aqmd3/SA220 ## waveform.wellKnownEvents: " << waveform.well_known_events();
    }

    waveform.set_elapsed_time( waveform.xmeta().initialXTimeSeconds - u5303_inject_timepoint_ );
    if ( c_acquisition_status_ )
        waveform.set_well_known_events( waveform.well_known_events() | adacquire::SignalObserver::wkEvent_AcqInProgress );
}

bool
task::handle_initial_setup()
{
    spDriver_ = std::make_shared< AqMD3 >();     // spDriver creation in the thread

    bool simulated = false;
    bool success = false;

    if ( auto p = getenv( "AcqirisOption" ) ) {
        if ( p && std::strcmp( p, "simulate" ) == 0 ) {
            const char * strInitOptions = "Simulate=true, DriverSetup= Model=SA230P";
            simulated = true;
            success = ( spDriver_->initWithOptions( "PXI40::0::0::INSTR", VI_FALSE, VI_TRUE, strInitOptions ) == VI_SUCCESS );
            ADDEBUG() << "################# AQMD3 SIMULATION MODE ##################: " << strInitOptions << " code: " << success;
        }
    }

    if ( !simulated ) {
        aqmd3::findResource findResource( true, true, "Simulate=false, DriverSetup= Model=SA230P" );
        if ( auto res = findResource( spDriver_, false ) ) {
            success = true;
        }
    }

    if ( success ) {
        simulated_ = simulated;

        if ( ! spDriver_->abort() )
            ADDEBUG() << "agmd3 abort failed";

        ident_ = std::make_shared< aqmd3controls::identify >();
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

        auto m( method_ );
        device::validate( spDriver_, *m );
        device::initial_setup( spDriver_, *m, ident().Options() );

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
    log( AqMD3_QueryBoardTemperature( spDriver()->session(), &temperature_ ), __FILE__,__LINE__ );
    log( AqMD3_QueryChannelTemperature ( spDriver()->session(), "Channel1", &channel_temperature_[ 0 ] ),__FILE__,__LINE__ );

    std::ostringstream o;
    o << temperature_ << ", Channel1: " << channel_temperature_[ 0 ];

    if ( ident_->Options().find( "INT" ) != std::string::npos ) {
        log( AqMD3_QueryChannelTemperature ( spDriver()->session(), "Channel2", &channel_temperature_[ 1 ] ),__FILE__,__LINE__ );
        o << ", Channel2: " << channel_temperature_[ 1 ];
    }

    for ( auto& reply: reply_handlers_ )
        reply( "Temperature", o.str() );

    // ADDEBUG() << "U5303A: " << ident_->SerialNumber() << "\tTemprature: " << o.str();

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
task::handle_prepare_for_run( const aqmd3controls::method t )
{
#ifndef NDEBUG
    ADDEBUG() << "=============== " << __FUNCTION__ << " ====================";
#endif
    if ( fsm_.process_event( fsm::Prepare() ) == boost::msm::back::HANDLED_FALSE )
        return false;

    c_acquisition_status_ = false;
    c_injection_requested_ = false;
    u5303_inject_timepoint_ = 0;

    auto m = std::make_shared< aqmd3controls::method >( t );

    device::validate( spDriver_, *m );
    device::initial_setup( spDriver_, *m, ident().Options() );

    if ( /* m.mode_ && */ simulated_ ) {
        m->device_method().samp_rate = spDriver()->SampleRate();
        simulator::instance()->setup( *m );
    }

    method_ = m;

    if ( m->device_method().TSR_enabled ) {
        return fsm_.process_event( fsm::TSRInitiate() ) == boost::msm::back::HANDLED_TRUE;
    } else {
        return fsm_.process_event( fsm::Initiate() ) == boost::msm::back::HANDLED_TRUE;
    }

}

bool
task::handle_protocol( const aqmd3controls::method m )
{
    auto t = std::make_shared< aqmd3controls::method >( m );
    device::setup( spDriver_, *t );

    if ( simulated_ )
        simulator::instance()->setup( *t );

    method_ = t;
    return true;
}


bool
task::handle_TSR_acquire()
{
    auto m( method_ );

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

    std::vector< std::shared_ptr< aqmd3controls::waveform > > vec;

    digitizer::readData( *spDriver(), *m, vec );
    spDriver_->TSRContinue();

    for ( auto& waveform: vec ) {
        // ==> set elapsed time for debugging
        set_time_since_inject( *waveform );
        // <==
        aqmd3controls::method m;
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

    auto m( method_ );

    if ( acquire() ) {

        using aqmd3controls::method;
        if ( waitForEndOfAcquisition( 3000 ) ) {

            auto epoch_time = std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::system_clock::now().time_since_epoch() ).count();

            if ( m->mode() == method::DigiMode::Digitizer ) { // digitizer

                int protocolIndex = pio_->protocol_number(); // <- hard wired protocol id

                if ( protocolIndex < 0 && simulated_ )
                	protocolIndex = simulator::instance()->protocol_number();

                if ( protocolIndex >= 0 )
                    m->setProtocolIndex( protocolIndex & 03, false ); // 2bits

                std::vector< std::shared_ptr< aqmd3controls::waveform > > vec;

                // read multiple waveform using mblock
                digitizer::readData( *spDriver(), *m, vec );

                if ( simulated_ )
                    simulator::instance()->touchup( vec, *m );

                for ( auto& waveform: vec ) {
                    set_time_since_inject( *waveform );         // <---------- INJECTION event set ------------
                    aqmd3controls::method m;
                    for ( auto& reply: waveform_handlers_ ) {
                        if ( reply( waveform.get(), nullptr, m ) )
                            handle_protocol( m );
                    }
                }

            } else { // average
                uint32_t events = darkCount_ ? adacquire::SignalObserver::wkEvent_DarkInProgress : 0;

                if ( m->device_method().pkd_enabled ) {
                    // PKD+AVG
                    auto pkd = std::make_shared< aqmd3controls::waveform >( ident_, events );
                    auto avg = std::make_shared< aqmd3controls::waveform >( ident_, events );
                    if ( readDataPkdAvg( *pkd, *avg, epoch_time ) ) {
                        set_time_since_inject( *pkd );          // <---------- INJECTION event set ------------
                        set_time_since_inject( *avg );          // <---------- INJECTION event set ------------
                        aqmd3controls::method t;
                        for ( auto& reply : waveform_handlers_ ) {
                            if ( reply( avg.get(), pkd.get(), t ) )
                                handle_protocol( t );
                        }
                        // auto dark( darkWaveform_ );
                        // if ( darkCount_ && --darkCount_ == 0 ) {
                        //     darkWaveform_ = avg;
                        //     for ( auto& reply: reply_handlers_ ) reply( "DarkAcquired", "" );
                        // }
                        // if ( dark ) {
                        //     assert(0);
                        //     // avg->darkSubtraction( *dark );
                        // }
                    }

                } else {
                    // AVERAGE
                    auto waveform = std::make_shared< aqmd3controls::waveform >( ident_, events );
                    if ( readData( *waveform ) ) {
                        set_time_since_inject( *waveform );     // <---------- INJECTION event set ------------
                        aqmd3controls::method t;
                        for ( auto& reply : waveform_handlers_ ) {
                            if ( reply( waveform.get(), nullptr, t ) )
                                handle_protocol( t );
                        }
                        auto dark( darkWaveform_ );
                        if ( darkCount_ && --darkCount_ == 0 ) {
                            darkWaveform_ = waveform;
                            for ( auto& reply: reply_handlers_ ) reply( "DarkAcquired", "" );
                        }
                        // if ( dark ) {
                        //     assert(0);
                        //     //waveform->darkSubtraction( *dark );
                        // }
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
    auto m( method_ );
    if ( (m->mode() == aqmd3controls::method::DigiMode::Averager) && simulated_ )
        return simulator::instance()->acquire();

    tp_acquire_ = std::chrono::system_clock::now();
    return device::acquire( spDriver_ );
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
        if ( method_->mode() == aqmd3controls::method::DigiMode::Averager )
            return simulator::instance()->waitForEndOfAcquisition();
    }

    return device::waitForEndOfAcquisition( spDriver_, timeout );
}

bool
task::readDataPkdAvg( aqmd3controls::waveform& pkd, aqmd3controls::waveform& avg, int64_t epoch_time )
{
    pkd.set_serialnumber( spDriver()->dataSerialNumber() );
    avg.set_serialnumber( spDriver()->dataSerialNumber() );
    set_time_since_inject( pkd );
    set_time_since_inject( avg );

    auto m( method_ );
    pkd.set_method( *m );
    avg.set_method( *m );

    if ( simulated_ ) {
        simulator::instance()->readDataPkdAvg( pkd, avg );
        pkd.set_epoch_time( epoch_time );
        avg.set_epoch_time( epoch_time ); // timeSinceEpoch_ = pkd.timeSinceEpoch_;
        pkd.xmeta().channelMode = aqmd3controls::PKD;
        avg.xmeta().channelMode = aqmd3controls::AVG;
        return true;
    }

    ADDEBUG() << "readDataPkdAvg";

    auto md3( spDriver_ );
    ViInt64 arraySize = 0;
    const int64_t recordSize = m->device_method().digitizer_nbr_of_s_to_acquire;
	if ( md3->QueryMinWaveformMemory( 32, 1, 0, recordSize, arraySize) ) {
        ViInt64 actualPoints = {0}, firstValidPoint;
        do { // PKD
            auto mblk = std::make_shared< adportable::mblock< int32_t > >( arraySize );
            ViInt64 addressLow = 0x00000000;
            ViInt32 addressHigh_Ch1 = 0x00000080; // To read the Peak Histogram on CH1
            md3->LogicDeviceReadIndirectInt32( "DpuA", addressHigh_Ch1, addressLow, m->device_method().nbr_of_s_to_acquire_
                                               , arraySize, mblk->data(), actualPoints, firstValidPoint );
            pkd.set_method( *m );
            pkd.xmeta().initialXTimeSeconds = md3->pkdTimestamp() * 1.0e-12; // ps -> s
            pkd.xmeta().actualAverages      = md3->pkdActualAverages();
            pkd.xmeta().actualPoints        = actualPoints;
            pkd.xmeta().xIncrement          = 1.0 / m->device_method().samp_rate;
            pkd.xmeta().initialXOffset      = m->device_method().delay_to_first_sample_; //  initialXOffset;
            pkd.xmeta().scaleFactor         = 1.0; // pkd
            pkd.xmeta().scaleOffset         = 0.0; // pkd
            pkd.xmeta().protocolIndex       = m->protocolIndex();
            pkd.xmeta().dataType            = 4;
            pkd.xmeta().firstValidPoint     = firstValidPoint;
            pkd.set_epoch_time( epoch_time );
            // pkd.set_epoch_time( std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::system_clock::now().time_since_epoch() ).count() );
            pkd.setData( mblk, firstValidPoint, actualPoints );

        } while ( 0 );

        do { // AVG
            auto mblk = std::make_shared< adportable::mblock< int32_t > >( arraySize );
            ViInt64 addressLow = 0x00000000;
            ViInt32 addressHigh_Ch2 = 0x00000090; // To read the accumulated raw data on CH2
            md3->LogicDeviceReadIndirectInt32( "DpuA", addressHigh_Ch2, addressLow, m->device_method().nbr_of_s_to_acquire_
                                               , arraySize, mblk->data(), actualPoints, firstValidPoint );
            avg.set_method( *m );
            avg.xmeta()                   = pkd.xmeta(); // copy
            avg.xmeta().actualPoints      = actualPoints;
            avg.xmeta().firstValidPoint   = firstValidPoint;
            avg.xmeta().dataType          = 4;
            avg.xmeta().scaleFactor       = 1.0; // m->device_method().front_end_range / 65536 / pkd.xmeta().actualAverages;
            avg.xmeta().scaleOffset       = m->device_method().front_end_offset; // scaleOffset;  <-- offset direct 0.1 -> 0.1; -0.1 -> -0.2
            avg.setData( mblk, firstValidPoint, actualPoints );

        } while ( 0 );
    }
    ADDEBUG() << std::make_pair( avg.xmeta().scaleFactor, avg.xmeta().scaleOffset )
              << ", actualPoints: " << avg.xmeta().actualPoints
              << ", xInc: " << avg.xmeta().xIncrement;
    // digitizer::readData32( *spDriver(), method_, pkd, "Channel1" );
    // pkd.xmeta().channelMode = aqmd3controls::PKD;

    // digitizer::readData32( *spDriver(), method_, avg, "Channel2" );
    // avg.xmeta().channelMode = aqmd3controls::AVG;

    return true;
}

bool
task::readData( aqmd3controls::waveform& data )
{
    data.set_serialnumber( spDriver()->dataSerialNumber() );

    if ( simulated_ ) {
        if ( simulator::instance()->readData( data ) ) {
            data.set_epoch_time( std::chrono::system_clock::now().time_since_epoch().count() ); // data.timeSinceEpoch_
            set_time_since_inject( data ); // ==> set elapsed time for debugging
            return true;
        }
    }
    //-------------------
    auto m( method_ );
    bool result( false );
    if ( m->mode() == aqmd3controls::method::DigiMode::Digitizer ) {
        result = digitizer::readData16( *spDriver(), *m, data );
    } else {
        result = digitizer::readData32( *spDriver(), *m, data, "Channel1" );
        data.xmeta().channelMode = aqmd3controls::AVG;
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


ViStatus
task::log( ViStatus rcode, const char * const file, int line, std::function< std::string() > details ) const
{
    spDriver_->syslog( rcode, file, line, details );
    return rcode;
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
bool
device::validate( std::shared_ptr< aqmd3::AqMD3 > md3, aqmd3controls::method& m )
{
    if ( std::find( ModelSA.begin(), ModelSA.end(), md3->Identify()->InstrumentModel() ) != ModelSA.end() ) {
        if ( !( adportable::compare<double>::is_equal( m.device_method().front_end_range, 0.5) ||
                adportable::compare<double>::is_equal( m.device_method().front_end_range , 2.5) ) ) {
            m.device_method().front_end_range  = 2.5; // 2.5V
        }
        m.device_method().samp_rate = 2.0e9;  // 2GSPS only
        return true;
    }
    return false;
}

bool
device::initial_setup( std::shared_ptr< aqmd3::AqMD3 > md3, const aqmd3controls::method& m, const std::string& options )
{
    ADDEBUG() << "##### initial_setup for '" << md3->Identify()->InstrumentModel() << "' #####";
    ADDEBUG() << "##### front_end_range: " << m.toJson();

    if ( !md3->ConfigureChannel( "Channel1"
                                , m.device_method().front_end_range
                                , m.device_method().front_end_offset
                                , AQMD3_VAL_VERTICAL_COUPLING_DC
                                , VI_TRUE ) ) {
        return false;
    }
    // trigger
    md3->clog( attribute< active_trigger_source >::set( *md3, "External1"), __FILE__, __LINE__ );
    md3->clog( attribute< trigger_level >::set( *md3, "External1", m.device_method().ext_trigger_level ), __FILE__, __LINE__ );
    md3->clog( attribute< trigger_slope >::set( *md3, "External1", AQMD3_VAL_TRIGGER_SLOPE_POSITIVE ), __FILE__, __LINE__ );
    md3->clog( attribute< trigger_coupling >::set( *md3, "External1", AQMD3_VAL_TRIGGER_COUPLING_DC ), __FILE__, __LINE__ );
    md3->clog( attribute< trigger_delay >::set( *md3, m.device_method().delay_to_first_sample_ ), __FILE__, __LINE__ );

    if ( m.mode() == aqmd3controls::method::DigiMode::Digitizer ) { // Digitizer

        ADDEBUG() << "##### --> digitizer mode options: " << options;
        if ( options.find( "TSR" ) != options.npos )
            attribute< tsr_enabled >::set( *md3, m.device_method().TSR_enabled );

        md3->setAcquisitionMode( AQMD3_VAL_ACQUISITION_MODE_NORMAL );
        md3->setAcquisitionRecordSize( m.device_method().nbr_of_s_to_acquire_ );
        md3->setAcquisitionNumRecordsToAcquire( m.device_method().nbr_records );

    } else { // Averager

        ADDEBUG() << "##### --> averager mode options: " << options;
        if ( options.find( "TSR" ) != options.npos )
            attribute< tsr_enabled >::set( *md3, false );
        if ( md3->Identify()->Options().find( "AVG" ) == std::string::npos )
            return false;

        md3->clog( aqmd3::attribute< aqmd3::sample_rate >::set( *md3, m.device_method().samp_rate ), __FILE__, __LINE__ );
        md3->clog( aqmd3::attribute< aqmd3::record_size >::set( *md3, m.device_method().nbr_of_s_to_acquire_ ), __FILE__, __LINE__ );
        md3->clog( aqmd3::attribute< aqmd3::acquisition_number_of_averages >::set( *md3, m.device_method().nbr_of_averages ), __FILE__, __LINE__ );
        md3->clog( aqmd3::attribute< aqmd3::acquisition_mode >::set( *md3, AQMD3_VAL_ACQUISITION_MODE_AVERAGER ), __FILE__, __LINE__ );
        md3->clog( aqmd3::attribute< aqmd3::channel_data_inversion_enabled >::set( *md3, "Channel1", m.device_method().invert_signal )
                   ,  __FILE__, __LINE__ );

        ViInt32 const blDigitalOffset = 0;
        ViInt32 const blPulseThreshold = 500;
        md3->clog( aqmd3::attribute< aqmd3::channel_baseline_correction_mode >::set( *md3, "Channel1", aqmd3::BASELINE_CORRECTION_MODE_CONTINUOUS )
                   , __FILE__, __LINE__ );
        md3->clog( aqmd3::attribute< aqmd3::channel_baseline_correction_digital_offset >::set( *md3, "Channel1", blDigitalOffset)
                   , __FILE__, __LINE__ );
        md3->clog( aqmd3::attribute< aqmd3::channel_baseline_correction_pulse_threshold >::set( *md3, "Channel1", blPulseThreshold)
                   , __FILE__, __LINE__ );
        md3->clog( aqmd3::attribute< aqmd3::channel_baseline_correction_pulse_polarity >::set( *md3, "Channel1"
                                                                                               , aqmd3::BASELINE_CORRECTION_PULSE_POLARITY_POSITIVE )
                   , __FILE__, __LINE__ );

        if ( m.device_method().pkd_enabled ) {
            if ( m.device_method().pkd_amplitude_accumulation_enabled ) // AmplitudeAccumulationEnabled==0)
                md3->LogicDeviceWriteRegisterInt32("DpuA", 0x33B4, 0x143511 ); // PKD - Amplitude mode
            else
                md3->LogicDeviceWriteRegisterInt32("DpuA", 0x33B4, 0x143515 ); // PKD - Count mode

            md3->LogicDeviceWriteRegisterInt32( "DpuA", 0x33B8, m.device_method().pkd_rising_delta|(m.device_method().pkd_falling_delta << 16));

            // Required to complete the PKD configuration
            md3->LogicDeviceWriteRegisterInt32( "DpuA", 0x3350, 0x00000027 ); //PKD configuration
        }
#if 0
        // PKD - POC
        if ( m.device_method().pkd_enabled && options.find( "PKD" ) != options.npos ) {
            ADINFO() << "##### PKD ON; Invert signal " << ( m.device_method().invert_signal ? "true" : "false" )
                     << "; Amplitude accum. " << (m.device_method().pkd_amplitude_accumulation_enabled ? "enabled" : "disabled");
            md3->clog( attribute< num_records_to_acquire >::set( *md3, int64_t( 1 ) ), __FILE__,__LINE__ );
            md3->clog( attribute< acquisition_mode >::set( *md3, AQMD3_VAL_ACQUISITION_MODE_PEAK_DETECTION ), __FILE__,__LINE__ );
            // Configure the data inversion mode - VI_FALSE (no data inversion) by default
            md3->clog( attribute< channel_data_inversion_enabled >::set( *md3
                                                                          , "Channel1"
                                                                          , bool( m.device_method().invert_signal ) ), __FILE__,__LINE__ );
            // Configure the accumulation enable mode: the peak value is stored (VI_TRUE) or the peak value is forced to '1' (VI_FALSE).
            md3->clog( attribute< peak_detection_amplitude_accumulation_enabled >::set(
                           *md3, "Channel1", m.device_method().pkd_amplitude_accumulation_enabled )
                       , __FILE__,__LINE__ );
            // Configure the RisingDelta and FallingDelta in LSB: define the amount by which two consecutive samples must differ to be
            // considered as rising/falling edge in the peak detection algorithm.
            md3->clog( attribute< peak_detection_rising_delta >::set( *md3, "Channel1", m.device_method().pkd_rising_delta ), __FILE__,__LINE__ );
            md3->clog( attribute< peak_detection_falling_delta >::set( *md3, "Channel1", m.device_method().pkd_falling_delta ), __FILE__,__LINE__ );
            md3->clog( attribute< record_size >::set( *md3, m.device_method().nbr_of_s_to_acquire_ ), __FILE__,__LINE__ );
            md3->clog( attribute< num_records_to_acquire >::set( *md3, int64_t( 1 ) ), __FILE__,__LINE__ );
            md3->clog( attribute< acquisition_number_of_averages >::set( *md3, m.device_method().nbr_of_averages ), __FILE__,__LINE__ );
        } else {
            ADINFO() << "##### AVG ON; Invert signal " << ( m.device_method().invert_signal ? "true" : "false" );
            md3->clog( attribute< num_records_to_acquire >::set( *md3, int64_t( 1 ) ), __FILE__,__LINE__ );
            md3->clog( attribute< acquisition_mode >::set( *md3, AQMD3_VAL_ACQUISITION_MODE_AVERAGER ), __FILE__,__LINE__ );
            md3->clog( attribute< channel_data_inversion_enabled >::set( *md3
                                                                        , "Channel1"
                                                                        , bool( m.device_method().invert_signal ) ), __FILE__,__LINE__ );
            md3->clog( attribute< record_size >::set( *md3, m.device_method().nbr_of_s_to_acquire_ ), __FILE__,__LINE__ );
            md3->clog( attribute< acquisition_number_of_averages >::set( *md3, m.device_method().nbr_of_averages ), __FILE__,__LINE__ );
        }
#endif
    }
    // ADTRACE() << "##### ACQUISITION_MODE : " << task.spDriver()->AcquisitionMode();
    md3->SelfCalibrate();

	return true;
}

bool
device::setup( std::shared_ptr< aqmd3::AqMD3 > md3, const aqmd3controls::method& m )
{
    // Don't forget environment variable: 'AQMD3_SKIP_CAL_REQUIRED_CHECKS=1'

    md3->setTriggerDelay( m.device_method().delay_to_first_sample_ );
    md3->setAcquisitionRecordSize( m.device_method().nbr_of_s_to_acquire_ );
    md3->setAcquisitionNumRecordsToAcquire( m.device_method().nbr_records );

    return true;
}

bool
device::acquire( std::shared_ptr< aqmd3::AqMD3 > md3 )
{
    return md3->AcquisitionInitiate();
}

bool
device::waitForEndOfAcquisition( std::shared_ptr< aqmd3::AqMD3 > md3, int timeout )
{
    auto tp = std::chrono::system_clock::now() + std::chrono::milliseconds( timeout );

    while( ! md3->isAcquisitionIdle() ) {
        if ( tp < std::chrono::system_clock::now() )
            return false;
    }
    return true;
}

/////////////
bool
digitizer::readData( AqMD3& md2, const aqmd3controls::method& m, std::vector< std::shared_ptr< aqmd3controls::waveform > >& vec )
{
    ViInt64 arraySize = 0;
    const int64_t recordSize = m.device_method().digitizer_nbr_of_s_to_acquire;
    const int64_t numRecords = m.device_method().nbr_records;

    if ( md2.clog(
             AqMD3_QueryMinWaveformMemory( md2.session(), 16, numRecords, 0, recordSize, &arraySize )
             , __FILE__, __LINE__ ) ) {

        auto mblk = std::make_shared< adportable::mblock<int16_t> >( arraySize );
        std::fill( mblk->data(), mblk->data() + arraySize, 0 );

        ViInt64 actualRecords(0); //, waveformArrayActualSize(0);
        std::vector<ViInt64> actualPoints( numRecords ), firstValidPoints( numRecords );
        std::vector<ViReal64> initialXOffset( numRecords ), initialXTimeSeconds( numRecords ), initialXTimeFraction( numRecords );
        ViReal64 xIncrement(0), scaleFactor(0), scaleOffset(0);

        if ( md2.clog( AqMD3_FetchMultiRecordWaveformInt16( md2.session()
                                                            , "Channel1"
                                                            , 0
                                                            , numRecords
                                                            , 0
                                                            , recordSize
                                                            , arraySize
                                                            , mblk->data()
                                                            //, &waveformArrayActualSize
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

                if ( auto data = std::make_shared< aqmd3controls::waveform >( md2.Identify(), md2.dataSerialNumber() ) ) {

                    auto& d = *data;

                    //d.device_method_ = m;
                    d.set_method( m );
                    d.xmeta().actualAverages = 0; // digitizer
                    d.xmeta().actualPoints = actualPoints[ iRecord ];
                    d.xmeta().initialXTimeSeconds = initialXTimeSeconds[ iRecord ] + initialXTimeFraction[ iRecord ];
                    d.xmeta().xIncrement  = xIncrement;
                    d.xmeta().initialXOffset = initialXOffset[ iRecord ];
                    d.xmeta().scaleFactor = scaleFactor;
                    d.xmeta().scaleOffset = scaleOffset;
                    d.xmeta().protocolIndex = m.protocolIndex();
                    d.xmeta().dataType = mblk->dataType();
                    d.set_epoch_time( std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::system_clock::now().time_since_epoch() ).count() );
                    d.setData( mblk, firstValidPoints[ iRecord ], d.xmeta().actualPoints );

                    // ADDEBUG() << "readData: " << d.method_.protocolIndex() << "/" << d.method_.protocols().size()
                    //           << " SF=" << scaleFactor << " SO=" << scaleOffset << " t=" << d.meta_.dataType
                    //           << " data[10]=" << boost::format( "%x" ) % mblk->data()[ 10 ];
#if !defined NDEBUG
                    static waveform_print printer;
                    printer( d, mblk->data() + firstValidPoints[ iRecord ] );
#endif
                    vec.emplace_back( data );
                }
            }
            return true;
        }
    }

    return false;
}

bool
digitizer::readData16( AqMD3& md2, const aqmd3controls::method& m, aqmd3controls::waveform& data )
{
    const int64_t recordSize = m.device_method().digitizer_nbr_of_s_to_acquire;
    ViInt64 const numRecords = 1;
    ViInt64 arraySize(0);

    if ( md2.clog(
             AqMD3_QueryMinWaveformMemory( md2.session(), 32, 1, 0, recordSize, &arraySize )
             , __FILE__, __LINE__ ) ) {

        ViInt32 actualAverages(0);
        ViInt64 actualPoints[numRecords] = {0}, firstValidPoint[numRecords] = {0};
        ViReal64 initialXTimeSeconds[numRecords] = {0}, initialXTimeFraction[numRecords] = {0};
        ViReal64 initialXOffset(0), xIncrement(0), scaleFactor(0), scaleOffset(0);

        //const auto& tp = task::instance()->tp_acquire();
        //uint64_t acquire_tp_count = std::chrono::duration_cast<std::chrono::nanoseconds>( tp.time_since_epoch() ).count();

		auto mblk = std::make_shared< adportable::mblock<int16_t> >( arraySize );

        if ( md2.clog( AqMD3_FetchWaveformInt16( md2.session()
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

            ADDEBUG() << __FUNCTION__;
            data.set_method( m );
            data.xmeta().actualAverages = actualAverages;
            data.xmeta().actualPoints = actualPoints[ 0 ];
            data.xmeta().initialXTimeSeconds = initialXTimeSeconds[ 0 ] + initialXTimeFraction[ 0 ];
            data.xmeta().xIncrement = xIncrement;
            data.xmeta().initialXOffset  = initialXOffset;
            data.xmeta().scaleFactor = scaleFactor;
            data.xmeta().scaleOffset = scaleOffset;
            data.xmeta().protocolIndex = m.protocolIndex();
            data.xmeta().dataType = 2;
            data.set_epoch_time( std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::system_clock::now().time_since_epoch() ).count() );
            data.setData( mblk, firstValidPoint[0], data.xmeta().actualPoints );

#if !defined NDEBUG
            static waveform_print printer;
            printer( data, mblk->data() + firstValidPoint[ 0 ] );
#endif
            return true;
        }
    }
    return false;
}

bool
digitizer::readData32( AqMD3& md2, const aqmd3controls::method& m, aqmd3controls::waveform& data
                       , const char * channel )
{
    ViInt64 constexpr numRecords = 1;
    const int64_t recordSize = m.device_method().digitizer_nbr_of_s_to_acquire;
    ViInt64 arraySize(0);

    if ( md2.clog(
             AqMD3_QueryMinWaveformMemory( md2.session(), 32, numRecords, 0, recordSize, &arraySize )
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

        if ( md2.clog( AqMD3_FetchAccumulatedWaveformInt32( md2.session()
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

            data.set_method( m );
            data.xmeta().actualAverages = actualAverages;
            data.xmeta().actualPoints = actualPoints[ 0 ];
            data.xmeta().initialXTimeSeconds = initialXTimeSeconds[ 0 ] + initialXTimeFraction[ 0 ];
            data.xmeta().xIncrement = xIncrement;
            data.xmeta().initialXOffset = initialXOffset;
            data.xmeta().scaleFactor = scaleFactor;
            data.xmeta().scaleOffset = scaleOffset;
            data.xmeta().protocolIndex = m.protocolIndex();
            data.xmeta().dataType = 4;
            data.xmeta().firstValidPoint = firstValidPoint[0];
            data.set_epoch_time( std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::system_clock::now().time_since_epoch() ).count() );
            data.setData( mblk, firstValidPoint[0], data.xmeta().actualPoints );

            ADDEBUG() << std::make_pair( data.xmeta().scaleFactor, data.xmeta().scaleOffset );
#if !defined NDEBUG
            static waveform_print printer;
            printer( data, mblk->data() + firstValidPoint[0] );
#endif
            return true;
        }
    }
    return false;
}
