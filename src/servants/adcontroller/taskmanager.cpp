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

#include "taskmanager.hpp"
#include "task.hpp"
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

    namespace singleton {
	typedef ACE_Singleton<iTaskManager, ACE_Recursive_Thread_Mutex> iTaskManager;
    }

    namespace internal {
	
        class TimeReceiver {
        public:
            TimeReceiver() {}
            int handle_input( ACE_HANDLE ) { return 0; }
            int handle_timeout( const ACE_Time_Value& tv, const void * arg) {
                return iTaskManager::instance()->handle_timeout( tv, arg );
            }
            int handle_close( ACE_HANDLE, ACE_Reactor_Mask ) { return 0; }
        };
	
    }
}

namespace adcontroller {
    template<> iTask * iTaskManager::get<iTask>() { return pTask_; }
}

///////////////////////////////////////////////////////////////////

iTaskManager::~iTaskManager()
{
    manager_terminate();
    ACE_Thread_Manager::instance()->wait();
    // following classes are stopped by ACE framework
    //delete pBroker_;
    //delete reactor_thread_;
}

iTaskManager::iTaskManager() : pTask_(0)
                             , reactor_thread_(0) 
                             , timerHandler_(0) 
{
    reactor_thread_ = new acewrapper::ReactorThread();
    reactor_thread_->spawn();
    pTask_ = new iTask( 8 );
}

// static
iTaskManager *
iTaskManager::instance()
{
    return singleton::iTaskManager::instance();
}

bool
iTaskManager::manager_initialize()
{
    if ( timerHandler_ == 0 ) {
	acewrapper::scoped_mutex_t<> lock( mutex_ );
	if ( timerHandler_ == 0 ) {
	    // initialize timer
	    timerHandler_ = new acewrapper::EventHandler< acewrapper::TimerReceiver<internal::TimeReceiver> >();
	    ACE_Reactor * reactor = iTaskManager::instance()->reactor();
	    reactor->schedule_timer( timerHandler_, 0, ACE_Time_Value(3), ACE_Time_Value(3) );
	}
	// activate task
	pTask_->open();
	return true;
    }
    return false;
}

void
iTaskManager::manager_terminate()
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
    pTask_->close();
    reactor_thread_->end_reactor_event_loop();
    reactor_thread_->join();
}

ACE_Reactor *
iTaskManager::reactor()
{ 
    return reactor_thread_ ? reactor_thread_->get_reactor() : 0;
}

int
iTaskManager::handle_timeout( const ACE_Time_Value& tv, const void * )
{
    using namespace adcontroller;
    
    if ( pTask_ )
        pTask_->putq( adcontroller::marshal< ACE_Time_Value >::put( tv, constants::MB_TIME_VALUE ) );
    
    return 0;
}
