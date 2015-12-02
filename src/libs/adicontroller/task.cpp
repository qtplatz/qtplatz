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
#include <acewrapper/udpeventreceiver.hpp>
#include <adportable/debug.hpp>
#include <boost/asio.hpp>
#include <boost/exception/all.hpp>
#include <stdexcept>
#include <mutex>
#include <thread>

namespace adicontroller {

    class task::impl : public sequ::fsm::handler {
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
        void sequ_fsm_state( bool, sequ::fsm::idState ) override;

        void handle_event_out( const char * data, size_t length, const boost::asio::ip::udp::endpoint& ep ) {
            std::string recv( data, length );
            auto pos = recv.find( "EVENTOUT 1" );
            if ( pos != recv.npos )
                signalInstEvents_( Instrument::instEventInjectOut );
        }

        boost::asio::io_service io_service_;
        std::vector< std::thread > threads_;

        sequ::fsm::controller fsm_;
        std::unique_ptr< acewrapper::udpEventReceiver > udpReceiver_;

    public:        
        boost::signals2::signal< void( Instrument::eInstEvent ) > signalInstEvents_;
        boost::signals2::signal< void( int ) > signalFSMAction_;
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
task::connect( signal_inst_events_t f )
{
    return impl_->signalInstEvents_.connect( f );
}

boost::signals2::connection
task::connect( signal_fsm_action_t f )
{
    return impl_->signalFSMAction_.connect( f );
}

//////////////////////////////////////////////////

task::impl::impl() : fsm_( this )
{
    udpReceiver_.reset( new acewrapper::udpEventReceiver( io_service_, 7125 ) );
    udpReceiver_->connect( std::bind( &task::impl::handle_event_out
                                      , this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );
}

task::impl::~impl()
{
}

void
task::impl::initialize()
{
    static std::once_flag flag;
    std::call_once( flag, [&](){
            threads_.push_back( std::thread( [&](){ io_service_.run(); } ) );
        });

    fsm_.stop();
}

void
task::impl::finalize()
{
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
task::impl::sequ_fsm_state( bool, sequ::fsm::idState state )
{
    ADDEBUG() << "sequ_fsm_state(" << state << ")";
}

