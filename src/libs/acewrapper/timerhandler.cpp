// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "timerhandler.h"
#pragma warning (disable: 4669)
#include <ace/Reactor.h>
#include <ace/Synch.h>
#pragma warning (default: 4669)

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
