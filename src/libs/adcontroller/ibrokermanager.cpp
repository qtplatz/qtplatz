//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "IBrokerManager.h"
#include "ibroker.h"
#include <boost/noncopyable.hpp>
#include <acewrapper/mutex.hpp>
#include <ace/Thread_Manager.h>
#include <ace/Reactor.h>
#include "task.h"

///////////////////////////////////////////////////////////////////

IBrokerManager::~IBrokerManager()
{
    delete pBroker_;
	delete reactor_;
}

IBrokerManager::IBrokerManager() : pBroker_(0)
                                 , reactor_(0) 
{
    pBroker_ = new iBroker();
}

bool
IBrokerManager::initialize()
{
	acewrapper::scoped_mutex_t<> lock( mutex_ );
    
	if ( reactor_ == 0 ) {
		reactor_ = new ACE_Reactor();
		ACE_Thread_Manager::instance()->spawn( ACE_THR_FUNC( reactor_thread_entry ), reinterpret_cast<void *>( this ) );

        task_.reset( new Task( reactor_ ) );
		task_->activate();
		return true;
	}
	return false;
}

void
IBrokerManager::terminate()
{
	if ( reactor_ )  {
		acewrapper::scoped_mutex_t<> lock( mutex_ );
		task_->deactivate();
		if ( reactor_ ) {
			reactor_->end_reactor_event_loop();
			// reactor_->reactor_event_loop_done();
		}
	}
}

void *
IBrokerManager::reactor_thread_entry( void * me )
{
	IBrokerManager * pThis = reinterpret_cast<IBrokerManager *>(me);
    if ( pThis ) {
        pThis->reactor_->owner( ACE_OS::thr_self() );
        pThis->run_event_loop();
    }
    return 0;
}

void
IBrokerManager::run_event_loop()
{
    while ( reactor_->reactor_event_loop_done() == 0 )
        reactor_->run_reactor_event_loop();
}
