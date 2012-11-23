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
// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "timerhandler.hpp"

#include <ace/Reactor.h>
#include <ace/Synch.h>

using namespace acewrapper;

TimerHandler::~TimerHandler()
{
    delete sema_;
}

TimerHandler::TimerHandler() : sema_(0)
{
}

void
TimerHandler::cancel( ACE_Reactor* reactor, ACE_Event_Handler * handler )
{
    if ( ! sema_ )
        sema_ = new ACE_Semaphore(1, USYNC_THREAD, 0, 0, 1); // binary semaphore, make it full already
    reactor->cancel_timer( handler, 0 );
}

int
TimerHandler::signal()
{
    if ( sema_ )
        return sema_->release();
    return (-1);
}

int
TimerHandler::wait()
{
    if ( sema_ )
        return sema_->acquire();
    return (-1);
}
