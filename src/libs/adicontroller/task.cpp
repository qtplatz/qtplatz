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
#include <acewrapper/udpeventreceiver.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adportable/debug.hpp>
#include <boost/asio.hpp>
#include <boost/exception/all.hpp>
#include <stdexcept>
#include <mutex>
#include <thread>

namespace adicontroller {

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
        void sequ_action_stop() override;
        void sequ_action_start() override;
        void sequ_action_inject() override;
        void sequ_fsm_state( bool, fsm::idState ) override;

        void handle_event_out( const char * data, size_t length, const boost::asio::ip::udp::endpoint& ep ) {
            std::string recv( data, length );
            auto pos = recv.find( "EVENTOUT 1" );
            if ( pos != recv.npos )
                signalInstEvents_( Instrument::instEventInjectOut );
        }

        std::vector< std::thread > threads_;
    public:

        boost::asio::io_service io_service_;
        fsm::controller fsm_;        
        std::chrono::steady_clock::time_point tp_uptime_;
        std::chrono::steady_clock::time_point tp_inject_;
        std::unique_ptr< SampleSequence > sequence_;
        std::unique_ptr< MasterObserver > masterObserver_;

        std::unique_ptr< acewrapper::udpEventReceiver > udpReceiver_;
        boost::signals2::signal< void( Instrument::eInstEvent ) > signalInstEvents_;
        boost::signals2::signal< void( Instrument::idFSMAction ) > signalFSMAction_;
    };

    std::unique_ptr< task > task::impl::instance_;
    std::mutex task::impl::mutex_;
}

using namespace adicontroller;

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

const std::chrono::steady_clock::time_point&
task::tp_uptime() const
{
    return impl_->tp_uptime_;
}

const std::chrono::steady_clock::time_point&
task::tp_inject() const
{
    return impl_->tp_inject_;
}

void
task::post( std::shared_ptr< SampleProcessor >& sp )
{
    (*impl_->sequence_) << sp;
}

SampleSequence *
task::sampleSequence()
{
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
task::fsmInject()
{
    impl_->fsm_.process_event( fsm::Inject() );
}

void
task::fsmErrorClear()
{
    impl_->fsm_.process_event( fsm::error_clear() );    
}


void
task::prepare_next_sample( const adcontrols::SampleRun& run, const adcontrols::ControlMethod::Method& method )
{
    auto pRun = std::make_shared< adcontrols::SampleRun >( run );
    auto pCM  = std:: make_shared< adcontrols::ControlMethod::Method >( method );

    if ( auto sp = std::make_shared< SampleProcessor >( pRun, pCM ) ) {

        sp->prepare_storage( impl_->masterObserver_.get() );

        post( sp );
        
    }
    
}

//////////////////////////////////////////////////

task::impl::impl() : fsm_( this )
                   , tp_uptime_( std::chrono::steady_clock::now() )
                   , tp_inject_( tp_uptime_ )
                   , sequence_( new SampleSequence )
                   , masterObserver_( new MasterObserver )
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
            
            threads_.push_back( std::thread( [&](){ io_service_.run(); } ) );
        });

    fsm_.stop();
}

void
task::impl::finalize()
{
    fsm_.stop();
    udpReceiver_.reset();
    io_service_.stop();
    for ( auto& t: threads_ )
        t.join();
}

void
task::impl::sequ_action_stop()
{
    ADDEBUG() << "sequ_action_stop";
}

void
task::impl::sequ_action_start()
{
    ADDEBUG() << "sequ_action_start";    
}

void
task::impl::sequ_action_inject()
{
    ADDEBUG() << "sequ_action_inject";    
}

void
task::impl::sequ_fsm_state( bool, fsm::idState state )
{
    ADDEBUG() << "sequ_fsm_state(" << state << ")";
}

