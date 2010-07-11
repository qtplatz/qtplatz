//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "eventreceiver.h"
#include <acewrapper/mcasthandler.h>
#include <acewrapper/dgramhandler.h>

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
    char buf[4096];
    ACE_INET_Addr from;
    int res = dgram.recv( buf, sizeof(buf), from );
    // emit
    return res;
}

int
QEventReceiver::handle_input(acewrapper::McastHandler& mcast, ACE_HANDLE )
{
    char buf[4096];
    ACE_INET_Addr from;
    int res = mcast.recv( buf, sizeof(buf), from );
    // emit
    return res;
}

int
QEventReceiver::handle_timeout( const ACE_Time_Value&, const void * )
{
    return 0;
}

int
QEventReceiver::handle_close( ACE_HANDLE, ACE_Reactor_Mask )
{
    return 0;
}
