// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "reactorthread.hpp"

#if defined _MSC_VER
#pragma warning (disable : 4996)
#endif
#include <ace/Thread_Manager.h>
#include <ace/Reactor.h>
#include <ace/Singleton.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Semaphore.h>
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
    if ( reactor_->reactor_event_loop_done() == 0 ) {
        reactor_->end_reactor_event_loop();
        sema_->acquire(); // end_event_loop() will release semaphore
    }
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
