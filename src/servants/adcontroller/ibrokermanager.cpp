/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "ibrokermanager.hpp"
#include "ibroker.hpp"
#include "message.hpp"
#include "constants.hpp"
#include "marshal.hpp"
#include <boost/noncopyable.hpp>
#include <acewrapper/mutex.hpp>
#include <acewrapper/reactorthread.hpp>
#include <acewrapper/timerhandler.hpp>
#include <acewrapper/eventhandler.hpp>
#include <ace/Thread_Manager.h>
#include <ace/Reactor.h>

//////////////////////////

using namespace adcontroller;

namespace adcontroller {
    namespace internal {
	
	class TimeReceiver {
	public:
	    TimeReceiver() {}
	    int handle_input( ACE_HANDLE ) { return 0; }
	    int handle_timeout( const ACE_Time_Value& tv, const void * arg) {
		return singleton::iBrokerManager::instance()->handle_timeout( tv, arg );
	    }
	    int handle_close( ACE_HANDLE, ACE_Reactor_Mask ) { return 0; }
	};
	
    }
}

namespace adcontroller {
    template<> iBroker * IBrokerManager::get<iBroker>() { return pBroker_; }
}
///////////////////////////////////////////////////////////////////

IBrokerManager::~IBrokerManager()
{
    manager_terminate();
    ACE_Thread_Manager::instance()->wait();
    // following classes are stopped by ACE framework
    //delete pBroker_;
    //delete reactor_thread_;
}

IBrokerManager::IBrokerManager() : pBroker_(0)
                                 , reactor_thread_(0) 
				 , timerHandler_(0) 
{
    reactor_thread_ = new acewrapper::ReactorThread();
    acewrapper::ReactorThread::spawn( reactor_thread_ );
    pBroker_ = new iBroker( 5 );
}

bool
IBrokerManager::manager_initialize()
{
    if ( timerHandler_ == 0 ) {
	acewrapper::scoped_mutex_t<> lock( mutex_ );
	if ( timerHandler_ == 0 ) {
	    // initialize timer
	    timerHandler_ = new acewrapper::EventHandler< acewrapper::TimerReceiver<internal::TimeReceiver> >();
	    ACE_Reactor * reactor = singleton::iBrokerManager::instance()->reactor();
	    reactor->schedule_timer( timerHandler_, 0, ACE_Time_Value(3), ACE_Time_Value(3) );
	}
	// activate task
	pBroker_->open();
	return true;
    }
    return false;
}

void
IBrokerManager::manager_terminate()
{
    if ( timerHandler_ ) {
	acewrapper::scoped_mutex_t<> lock( mutex_ );
	if ( timerHandler_ ) {
            timerHandler_->cancel( reactor(), timerHandler_ );
            timerHandler_->wait();
            delete timerHandler_;
	    timerHandler_ = 0;
	}
    }
    pBroker_->close();
    reactor_thread_->terminate();
}

ACE_Reactor *
IBrokerManager::reactor()
{ 
    return reactor_thread_ ? reactor_thread_->get_reactor() : 0;
}

int
IBrokerManager::handle_timeout( const ACE_Time_Value& tv, const void * )
{
    using namespace adcontroller;
    
    if ( pBroker_ )
        pBroker_->putq( adcontroller::marshal< ACE_Time_Value >::put( tv, constants::MB_TIME_VALUE ) );
    
    return 0;
}
