//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "task.h"
#include <acewrapper/messageblock.h>
#include <acewrapper/mutex.hpp>
#include <acewrapper/reactorthread.h>

Task::~Task()
{
}

Task::Task()
{
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
	for ( ;; ) {

		ACE_Message_Block * mblk = 0;

		if ( this->getq( mblk ) == (-1) ) {
			if ( errno == ESHUTDOWN )
				ACE_ERROR_RETURN((LM_ERROR, "(%t) queue is deactivated\n"), 0);
			else
				ACE_ERROR_RETURN((LM_ERROR, "(%t) %p\n", "putq"), -1);
		}
		if ( mblk->msg_type() == ACE_Message_Block::MB_HANGUP ) {
			this->putq( mblk ); // forward the request to any peer threads
			break;
		}
		//doit( mblk );
		ACE_Message_Block::release( mblk );
	}
	return 0;
}
