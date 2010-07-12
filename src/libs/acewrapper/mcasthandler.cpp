//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "mcasthandler.h"
#include <ace/SOCK_Dgram_Mcast.h>
#include <ace/INET_Addr.h>
#include <ace/Default_Constants.h>

using namespace acewrapper;

McastHandler::McastHandler()
{
   sock_addr_ = ACE_INET_Addr( ACE_DEFAULT_MULTICAST_PORT, ACE_DEFAULT_MULTICAST_ADDR );
}

ACE_HANDLE
McastHandler::get_handle() const
{
   return sock_mcast_.get_handle();
}

bool
McastHandler::open( u_short port )
{
    if ( port )
        sock_addr_.set_port_number(port);

    if ( sock_mcast_.join( sock_addr_ ) == (-1) )
        return false;
    return true;
}

bool
McastHandler::send( const char * pbuf, ssize_t size )
{
	ssize_t r = sock_mcast_.send( pbuf, size );
	return r == size;
}

int
McastHandler::recv( char * pbuf, int bufsize, ACE_INET_Addr& remote_addr)
{
   return sock_mcast_.recv( pbuf, bufsize, remote_addr );
}

