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
#include <ace/Thread_Manager.h>
#include <ace/Reactor.h>
#include <ace/Singleton.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Semaphore.h>
#include <ace/OS.h>
#include <acewrapper/mutex.hpp>
#include <adportable/debug.hpp>

#if defined _DEBUG
# include <iostream>
#endif

using namespace acewrapper;

ReactorThread::~ReactorThread()
{
    delete reactor_;
}

ReactorThread::ReactorThread() : reactor_( new ACE_Reactor )
                               , t_handle_( 0 )
{
}

bool
ReactorThread::end_reactor_event_loop()
{
    if ( reactor_->reactor_event_loop_done() != 0 )
        return false;
    reactor_->end_reactor_event_loop();
    return true;
}

bool
ReactorThread::spawn()
{
    if ( t_handle_ )
        return false;
    ACE_Thread_Manager::instance()->spawn( ACE_THR_FUNC(thread_entry), this );
    return true;
}

// satic
// void
// ReactorThread::spawn( ReactorThread * pThis )
// {
//     ACE_Thread_Manager * mgr = ACE_Thread_Manager::instance();
//     mgr->spawn(ACE_THR_FUNC( thread_entry ), reinterpret_cast<void *>( pThis ) );
// }

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
    t_handle_ = ACE_Thread::self();
	while ( reactor_->reactor_event_loop_done() == 0 )
		reactor_->run_reactor_event_loop();
}

ACE_Reactor *
ReactorThread::get_reactor()
{ 
    return reactor_;
}

bool
ReactorThread::join()
{
    if ( t_handle_ ) {
        if ( ACE_Thread_Manager::instance()->join( t_handle_ ) != 0 )
            adportable::debug(__FILE__, __LINE__) << "Reactor::join(" << t_handle_ << ") call failed: " << ACE_OS::strerror( errno );
    }
    t_handle_ = 0;
    return true;
}
