//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dgramhandler.h"
#include "callback.h"
#include <assert.h>
#include <ace/Reactor.h>

using namespace acewrapper;

static Callback __callback__;

DgramHandler::DgramHandler()
{
}


ACE_HANDLE
DgramHandler::get_handle() const
{
   return sock_dgram_.get_handle();
}

//
bool
DgramHandler::send( const char * pbuf, ssize_t octets, const ACE_INET_Addr& to )
{
    return sock_dgram_.send(pbuf, octets, const_cast<ACE_INET_Addr&>(to)) == octets;
}

int
DgramHandler::recv( char * pbuf, int bufsize, ACE_INET_Addr& remote_addr )
{
    return sock_dgram_.recv(pbuf, bufsize, remote_addr);
}

bool
DgramHandler::open( u_short port )
{
    if ( sock_dgram_.get_handle() )
        return false; // already open

    if ( port )
        sock_addr_ = ACE_INET_Addr( port, "0.0.0.0" );
   
    if ( sock_dgram_.open( sock_addr_ ) == (-1) ) {
        return false;
    }
    return true;
}

