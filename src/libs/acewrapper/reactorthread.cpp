// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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
#include <ace/Reactor.h>
#include <adportable/debug.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#if defined _DEBUG
# include <iostream>
#endif

using namespace acewrapper;

ReactorThread::~ReactorThread()
{
    delete reactor_;
}

ReactorThread::ReactorThread() : reactor_( new ACE_Reactor )
                               , thread_( 0 )
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
    if ( thread_ )
        return false;
    thread_ = new boost::thread( boost::bind( &ReactorThread::run_event_loop, this ) );
    return true;
}

void
ReactorThread::run_event_loop()
{
    reactor_->owner( ACE_OS::thr_self() );
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
    if ( thread_ )
		thread_->join();
    return true;
}
