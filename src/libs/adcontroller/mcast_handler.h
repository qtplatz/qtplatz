// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <ace/Event_Handler.h>
#include <string>

namespace acewrapper {
	class McastHandler;
}

class ACE_Time_Value;

class mcast_handler {
	std::string ior_;
public:
	~mcast_handler(void);
	mcast_handler(void);

	const std::string & ior() { return ior_; }
	void ior( const std::string& ior ) { ior_ = ior; }

	int handle_input( acewrapper::McastHandler&, ACE_HANDLE );
    int handle_timeout( const ACE_Time_Value&, const void * );
    int handle_close( ACE_HANDLE, ACE_Reactor_Mask );
};
