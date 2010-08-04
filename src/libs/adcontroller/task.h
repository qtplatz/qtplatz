// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#pragma warning (disable: 4996)
# include <ace/Task.h>
#pragma warning (default: 4996)

class ACE_Recursive_Thread_Mutex;

///////////////////////////

class Task : public ACE_Task<ACE_SYNCH> {
public:
	~Task();
	Task();

	bool activate();
    bool deactivate();

	virtual int handle_input( ACE_HANDLE );
	virtual int svc();
};

