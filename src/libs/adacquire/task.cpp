/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "task.hpp"
#include "automaton.hpp"
#include "constants.hpp"
#include "masterobserver.hpp"
#include "sampleprocessor.hpp"
#include "samplesequence.hpp"
#include "time_event_processor.hpp"
#include <acewrapper/udpeventreceiver.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adportable/debug.hpp>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/exception/all.hpp>
#include <boost/msm/back/tools.hpp>
#include <stdexcept>
#include <mutex>
#include <thread>

namespace adacquire {

    class task::impl : public fsm::handler {
    public:
        impl();
        ~impl();
        void initialize();
        void finalize();
        
        static std::unique_ptr< task > instance_;
        static std::mutex mutex_;

    private:
        // fsm::handler
        void fsm_action_stop() override;
        void fsm_action_start() override;
        void fsm_action_ready() override;
        void fsm_action_inject() override;
        void fsm_action_complete() override;
        void fsm_state( bool, fsm::idState, int ) override;
        void fsm_no_transition( int state ) override;
        void fsm_exception_caught( const char *, const std::exception& ) override;

        void handle_event_out( const char * data, size_t length, const boost::asio::ip::udp::endpoint& ep ) {
            std::string recv( data, length );
            auto pos = recv.find( "EVENTOUT 1" );
            if ( pos != recv.npos )
                signalInstEvents_( Instrument::instEventInjectOut );
        }

        void handle_timeout( const boost::system::error_code& ec );

        std::vector< std::thread > threads_;
    public:
        boost::asio::io_service io_service_;
        fsm::controller fsm_;        
        task::this_clock_t::time_point tp_uptime_;
        task::this_clock_t::time_point tp_inject_;
        std::unique_ptr< SampleSequence > sequence_;
        std::shared_ptr< MasterObserver > masterObserver_;

        std::unique_ptr< acewrapper::udpEventReceiver > udpReceiver_;
        boost::signals2::signal< void( Instrument::eInstEvent ) > signalInstEvents_;
        boost::signals2::signal< fsm_action_t > signalFSMAction_;
        boost::signals2::signal< fsm_state_changed_t > signalFSMStateChanged_;
        boost::signals2::signal< periodic_timer_t > signal_periodic_timer_;
        time_event_handler_t time_event_handler_;
        
        boost::asio::deadline_timer timer_;
        double methodTime_;
        bool inject_triggered_;
        uint32_t sequence_warning_count_;
        static Instrument::eInstStatus instStatus( int id_state );
        std::unique_ptr< time_event_processor > time_event_processor_;
    };

    std::unique_ptr< task > task::impl::instance_;
    std::mutex task::impl::mutex_;
}

using namespace adacquire;

task *
task::instance()
{
    static std::once_flag flag;
    std::call_once( flag, [](){ impl::instance_.reset( new task() ); } );

    return impl::instance_.get();
}

task::~task()
{
}

task::task() : impl_( new impl() )
{
}

boost::asio::io_service&
task::io_service()
{
    return impl_->io_service_;
}

void
task::initialize()
{
    impl_->initialize();
}

void
task::finalize()
{
    impl_->finalize();
}

boost::signals2::connection
task::connect_inst_events( signal_inst_events_t f )
{
    return impl_->signalInstEvents_.connect( f );
}

boost::signals2::connection
task::connect_fsm_action( signal_fsm_action_t f ) 
{
    return impl_->signalFSMAction_.connect( f );
}

boost::signals2::connection
task::connect_fsm_state( signal_fsm_state_changed_t f )
{
    return impl_->signalFSMStateChanged_.connect( f );    
}

boost::signals2::connection
task::connect_periodic_timer( signal_periodic_timer_t f )
{
	return impl_->signal_periodic_timer_.connect( f );
}

boost::signals2::connection
task::register_time_event_handler( const time_event_handler_t::slot_type& subscriber )
{
    return impl_->time_event_handler_.connect( subscriber );
}

const task::this_clock_t::time_point&
task::tp_uptime() const
{
    return impl_->tp_uptime_;
}

task::this_clock_t::time_point
task::tp_inject() const
{
    return impl_->tp_inject_;
}

void
task::post( std::shared_ptr< SampleProcessor > sp )
{
    std::lock_guard< std::mutex > lock( impl::mutex_ );
    (*impl_->sequence_) << sp;

    if ( ! impl_->time_event_processor_ )
        impl_->time_event_processor_ = std::make_unique< time_event_processor >();

    impl_->time_event_processor_->setControlMethod( sp->controlMethod(), sp->sampleRun()->methodTime() );
    impl_->time_event_processor_->action_start();
}

std::shared_ptr< SampleProcessor >
task::deque()
{
    std::lock_guard< std::mutex > lock( impl::mutex_ );
    return impl_->sequence_->deque();
}

const SampleSequence *
task::sampleSequence() const
{
    std::lock_guard< std::mutex > lock( impl::mutex_ );
    return impl_->sequence_.get();
}

MasterObserver *
task::masterObserver()
{
    return impl_->masterObserver_.get();
}

void
task::fsmStop()
{
    impl_->fsm_.process_event( fsm::Stop() );
}

void
task::fsmStart()
{
    impl_->fsm_.process_event( fsm::Start() );
}

void
task::fsmReady()
{
    impl_->fsm_.process_event( fsm::Ready() );
}

void
task::fsmInject()
{
    ADDEBUG() << "############### fsmInject #############";
    impl_->fsm_.process_event( fsm::Inject() );
}

void
task::fsmErrorClear()
{
    impl_->fsm_.process_event( fsm::error_clear() );    
}

void
task::prepare_next_sample( std::shared_ptr< adcontrols::SampleRun >& run, const adcontrols::ControlMethod::Method& method )
{
    auto pCM  = std:: make_shared< adcontrols::ControlMethod::Method >( method );

    auto clone = std::make_shared< adcontrols::SampleRun >( *run );
    if ( auto sp = std::make_shared< SampleProcessor >( clone, pCM ) ) {

        sp->prepare_storage( impl_->masterObserver_.get() );

        post( sp );

        impl_->methodTime_ = run->methodTime();
    }
#if ! defined NDEBUG
    ADDEBUG() << __FUNCTION__ << impl_->sequence_->size() << "; " << run->filePrefix() << " Length: " << impl_->methodTime_;
#endif
}

void
task::handle_write( const boost::uuids::uuid& uuid, std::shared_ptr< adacquire::SignalObserver::DataWriter > dw )
{
    std::lock_guard< std::mutex > lock( impl::mutex_ );

    if ( impl_->sequence_->size() == 0 && impl_->sequence_warning_count_++ == 0 )
        ADDEBUG() << "handle_write -- no sample processor in sample sequence";

#if ! defined NDEBUG && 0
    ADDEBUG() << "handle_write(" << uuid << ")";
#endif
    
    for ( auto& sampleprocessor : *impl_->sequence_ ) {

        sampleprocessor->write( uuid, *dw );
        impl_->sequence_warning_count_ = 0;

    }
    
}

adacquire::Instrument::eInstStatus
task::currentState() const
{
    return impl_->instStatus( impl_->fsm_.current_state()[0] );
}

//////////////////////////////////////////////////

task::impl::impl() : fsm_( this )
                   , tp_uptime_( std::chrono::system_clock::now() )
                   , tp_inject_( tp_uptime_ )
                   , sequence_( new SampleSequence )
                   , masterObserver_( std::make_shared<MasterObserver>() )
                   , timer_( io_service_ )
                   , methodTime_( 60.0 )
                   , inject_triggered_( false )
                   , sequence_warning_count_( 0 )
{
}

task::impl::~impl()
{
}

void
task::impl::initialize()
{
    static std::once_flag flag;
    std::call_once( flag, [&](){

            // Injection event listener via UDP port 7125 start
            udpReceiver_.reset( new acewrapper::udpEventReceiver( io_service_, 7125 ) );
            udpReceiver_->connect( std::bind( &task::impl::handle_event_out
                                              , this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );
            
            timer_.expires_from_now( boost::posix_time::seconds( 1 ) );
            timer_.async_wait( boost::bind( &impl::handle_timeout, this, boost::asio::placeholders::error ) );
            
            threads_.push_back( std::thread( [&](){ io_service_.run(); } ) );
        });

    fsm_.stop();
}

void
task::impl::finalize()
{
    fsm_.stop();
    sequence_->clear();
    udpReceiver_.reset();
    io_service_.stop();
    for ( auto& t: threads_ )
        t.join();
}

void
task::impl::fsm_action_stop()
{
    inject_triggered_ = false;
    signalFSMAction_( Instrument::fsmStop );
}

void
task::impl::fsm_action_start()
{
    inject_triggered_ = false;
    signalFSMAction_( Instrument::fsmStart );
}

void
task::impl::fsm_action_ready()
{
    inject_triggered_ = false;
    signalFSMAction_( Instrument::fsmReady );
}

void
task::impl::fsm_action_inject()
{
    tp_inject_ = std::chrono::system_clock::now();
    inject_triggered_ = true;

    signalFSMAction_( Instrument::fsmInject );

    // starting timed event triggers
    if ( time_event_processor_ )
        time_event_processor_->action_inject( tp_inject_ );
}

void
task::impl::fsm_action_complete()
{
    inject_triggered_ = false;
    signalFSMAction_( Instrument::fsmComplete );
}

void
task::impl::fsm_state( bool enter, fsm::idState state, int id_state )
{
    Instrument::eInstStatus st( Instrument::eNothing );
    switch( state ) {
    case fsm::idStopped:                st = Instrument::eStop; break;
    case fsm::idPreparingForRun:        st = Instrument::ePreparingForRun; break;
    case fsm::idWaitForContactClosure:  st = Instrument::eWaitingForContactClosure; break;
    case fsm::idRunning:                st = Instrument::eRunning; break;
    case fsm::idDormant:                st = Instrument::eStandBy; break;
    }
    signalFSMStateChanged_( enter, id_state, st );
}

void
task::impl::fsm_no_transition( int state )
{
    typedef boost::msm::back::recursive_get_transition_table< fsm::controller >::type recursive_stt;
    typedef boost::msm::back::generate_state_set<recursive_stt>::type all_states;
    
    std::string name;
    
    if ( boost::msm::back::get_state_id< recursive_stt, fsm::controller::Stopped >::value == state )
        name = "fsm::controller::Stopped";
    else if ( boost::msm::back::get_state_id< recursive_stt, fsm::controller::PreparingForRun >::value == state )
        name = "fsm::controller::PreparingForRun";
    else if ( boost::msm::back::get_state_id< recursive_stt, fsm::controller::WaitForContactClosure>::value == state )
        name = "fsm::controller::WaitForContactClosure";
    else if ( boost::msm::back::get_state_id< recursive_stt, fsm::controller::Running >::value == state )
        name = "fsm::controller::Running";
    else if ( boost::msm::back::get_state_id< recursive_stt, fsm::controller::Dormant >::value == state )
        name = "fsm::controller::Dormant";
    else
        boost::mpl::for_each<all_states,boost::msm::wrap<boost::mpl::placeholders::_1> >(boost::msm::back::get_state_name<recursive_stt>(name, state));
    
    // ADDEBUG() << "##### no transition from state (" << state << ") " << name;
}

void
task::impl::fsm_exception_caught( const char * name, const std::exception& ex )
{
    ADDEBUG() << "##### exception_caught " << name << "; " << ex.what();
}

Instrument::eInstStatus
task::impl::instStatus( int id_state )
{
#if defined _MSC_VER
    typedef boost::msm::back::recursive_get_transition_table< fsm::controller >::type recursive_stt;
    typedef boost::msm::back::generate_state_set<recursive_stt>::type all_states;
#else
    typedef typename boost::msm::back::recursive_get_transition_table< fsm::controller >::type recursive_stt;
    typedef typename boost::msm::back::generate_state_set<recursive_stt>::type all_states;
#endif
    if ( boost::msm::back::get_state_id< recursive_stt, fsm::controller::Stopped >::value == id_state )
        return Instrument::eStop;
    if ( boost::msm::back::get_state_id< recursive_stt, fsm::controller::PreparingForRun >::value == id_state )
        return Instrument::ePreparingForRun;
    if ( boost::msm::back::get_state_id< recursive_stt, fsm::controller::WaitForContactClosure>::value == id_state )
        return Instrument::eReadyForRun;
    if ( boost::msm::back::get_state_id< recursive_stt, fsm::controller::Running >::value == id_state )
        return Instrument::eRunning;
    if ( boost::msm::back::get_state_id< recursive_stt, fsm::controller::Dormant >::value == id_state )
        return Instrument::eStandBy;
    
    std::string name;
    boost::mpl::for_each<all_states,boost::msm::wrap<boost::mpl::placeholders::_1> >(boost::msm::back::get_state_name<recursive_stt>(name, id_state));
    ADDEBUG() << "Status: " << id_state << ": " << name;
    
    return Instrument::eNothing;
}

void
task::impl:: handle_timeout( const boost::system::error_code& ec )
{
    if ( !ec ) {

        if ( inject_triggered_ ) {

            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::system_clock::now() - tp_inject_ ).count();
			double elapsed_time = double(ns) * 1.0e-9; 
            
            signal_periodic_timer_( elapsed_time );
            
            if ( elapsed_time >= methodTime_ )
                fsm_.process_event( fsm::Complete() );
		} 

        timer_.expires_from_now( boost::posix_time::millisec( 100 ) );
        timer_.async_wait( boost::bind( &impl::handle_timeout, this, boost::asio::placeholders::error ) );
    }
}

void
task::time_event_trigger( std::shared_ptr< const adcontrols::ControlMethod::TimedEvents > tt
                          , adcontrols::ControlMethod::const_time_event_iterator begin
                          , adcontrols::ControlMethod::const_time_event_iterator end )
{
    impl_->time_event_handler_( tt, begin, end );
}
