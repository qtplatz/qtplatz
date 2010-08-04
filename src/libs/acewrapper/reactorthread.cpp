//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "reactorthread.h"

#pragma warning (disable : 4996)
#include <ace/Thread_Manager.h>
#include <ace/Reactor.h>
#include <ace/Singleton.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Semaphore.h>
#pragma warning (default: 4996)
#include <acewrapper/mutex.hpp>

#if defined _DEBUG
# include <iostream>
#endif

using namespace acewrapper;

ReactorThread::~ReactorThread()
{
    delete reactor_;
}

ReactorThread::ReactorThread() : reactor_(0)
                               , sema_(0) 
{
    reactor_ = new ACE_Reactor();
	sema_ = new ACE_Semaphore(0, USYNC_THREAD, 0, 0, 1); // binary semaphore, with count = 0
}

void
ReactorThread::terminate()
{
	reactor_->end_reactor_event_loop();
	sema_->acquire(); // end_event_loop() will release semaphore
}

// satic
void
ReactorThread::spawn( ReactorThread * pThis )
{
    ACE_Thread_Manager * mgr = ACE_Thread_Manager::instance();
    mgr->spawn(ACE_THR_FUNC( thread_entry ), reinterpret_cast<void *>( pThis ) );
}

void *
ReactorThread::thread_entry( void * me )
{
    ReactorThread * pThis = reinterpret_cast<ReactorThread *>(me);
    if ( pThis ) {
        pThis->reactor_->owner( ACE_OS::thr_self() );
        pThis->run_event_loop();
    }
    return 0;
}

void
ReactorThread::run_event_loop()
{
	while ( reactor_->reactor_event_loop_done() == 0 )
		reactor_->run_reactor_event_loop();
	sema_->release();
}


ACE_Reactor *
ReactorThread::get_reactor()
{ 
    return reactor_;
}
