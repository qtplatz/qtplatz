//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "eventreceiver.h"
#include <acewrapper/mcasthandler.h>
#include <acewrapper/dgramhandler.h>
#include <ace/Message_Block.h>
#include <iostream>
#include <acewrapper/ace_string.h>

QEventReceiver::QEventReceiver(QObject *parent) : QObject(parent)
{
}

int
QEventReceiver::handle_input( ACE_HANDLE )
{
    return 0;
}

int
QEventReceiver::handle_input(acewrapper::DgramHandler& dgram, ACE_HANDLE )
{
   // fix me
    static char buf[4096];
    static ACE_INET_Addr from;
    int res = dgram.recv( buf, sizeof(buf), from );

    // ACE_Message_Block * mb = new ACE_Message_Block( res + sizeof(ACE_INET_Addr) );
    std::string fromaddr = acewrapper::string( from );
    std::cout << "handle_input dgram (" << res << ")" << buf << fromaddr << std::endl;
    emit signal_dgram_input( buf, res, &from );

    return res;
}

int
QEventReceiver::handle_input(acewrapper::McastHandler& mcast, ACE_HANDLE )
{
    static char buf[4096];
    static ACE_INET_Addr from;
    int res = mcast.recv( buf, sizeof(buf), from );

    std::string fromaddr = acewrapper::string( from );
    std::cout << "handle_input mcast (" << res << ")" << buf << fromaddr << std::endl;

    // emit signal_mcast_input( buf, res, &from );
   return res;
}

int
QEventReceiver::handle_timeout( const ACE_Time_Value& tv, const void * )
{
   std::cout << "handle_timeout" << std::endl;
   // emit signal_timeout( &tv );
   return 0;
}

int
QEventReceiver::handle_close( ACE_HANDLE, ACE_Reactor_Mask )
{
    return 0;
}
