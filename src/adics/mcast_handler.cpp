//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "mcast_handler.h"
#include <ace/INET_Addr.h>
#include <acewrapper/mcasthandler.h>

mcast_handler::mcast_handler(void)
{
}

mcast_handler::~mcast_handler(void)
{
}

int
mcast_handler::handle_input( acewrapper::McastHandler& mcast, ACE_HANDLE )
{
   char rbuf[128];
   ACE_INET_Addr client;
   if ( mcast.recv( rbuf, sizeof(rbuf), client ) ) {
	   if ( strncmp( rbuf, "ior?", 4 ) == 0 )
		   mcast.send( ior_.c_str(), ior_.size() + 1 );
   }
   return 0;
}

int
mcast_handler::handle_timeout( const ACE_Time_Value&, const void * )
{
	return 0;
}

int
mcast_handler::handle_close( ACE_HANDLE, ACE_Reactor_Mask )
{
	return 0;
}
