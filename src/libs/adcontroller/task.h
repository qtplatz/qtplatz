// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <ace/Task.h>
#include <ace/Message_Queue.h>

class ACE_Recursive_Thread_Mutex;
class ACE_Notification_Strategy;

///////////////////////////

class Task : public ACE_Task<ACE_SYNCH> {
public:
	~Task();
	Task( ACE_Reactor * );

	bool activate();
    bool deactivate();

	virtual int handle_input( ACE_HANDLE );
	virtual int svc();
      
private:
	ACE_Message_Queue<ACE_SYNCH> msgq_;
	ACE_Notification_Strategy* notification_strategy_;
};

