//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "task.h"
#include <acewrapper/messageblock.h>
#include <acewrapper/mutex.hpp>
#include <acewrapper/reactorthread.h>
#include <ace/Reactor.h>
#include <ace/Reactor_Notification_Strategy.h>

Task::~Task()
{
	delete notification_strategy_;
}

Task::Task( ACE_Reactor * reactor ) : notification_strategy_(0)
{
	notification_strategy_ = new ACE_Reactor_Notification_Strategy( reactor, this, ACE_Event_Handler::READ_MASK );
	msg_queue()->notification_strategy( notification_strategy_ );
}

bool
Task::activate()
{
   msg_queue()->activate();
   return true;
}

bool
Task::deactivate()
{
   msg_queue()->deactivate();
   return true;
}

// virtual
int
Task::handle_input( ACE_HANDLE )
{
	ACE_Message_Block *mb;
	ACE_Time_Value zero( ACE_Time_Value::zero );
	if ( this->getq(mb, &zero) == -1 ) {
		ACE_ERROR((LM_ERROR, "(%t) %p\n", "dequeue_head"));
	} else {
		ACE_Message_Block::release(mb);
	}
	return 0;
}

int
Task::svc()
{
	int i = 0;

	for ( ;; ) {
		ACE_Message_Block * mblk = 0;
		if ( msgq_.dequeue_head(mblk) >= 0 ) {

			mblk->msg_priority(i++);
			// forward
			if ( this->putq(mblk) == -1 ) {
				if ( errno == ESHUTDOWN )
					ACE_ERROR_RETURN((LM_ERROR, "(%t) queue is deactivated\n"), 0);
				else
					ACE_ERROR_RETURN((LM_ERROR, "(%t) %p\n", "putq"), -1);
			}
		}			
	}
	ACE_NOTREACHED(return 0);
}
