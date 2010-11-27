//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "eventreceiver.h"
#include <acewrapper/mcasthandler.h>
#include <acewrapper/dgramhandler.h>
#pragma warning(disable:4996)
#include <ace/Message_Block.h>
#pragma warning(default:4996)
#include <iostream>
#include <acewrapper/ace_string.h>
#include <acewrapper/outputcdr.h>
#include <adportable/protocollifecycle.h>
#include <acewrapper/lifecycle_frame_serializer.h>
#include <boost/exception/all.hpp>

QEventReceiver::QEventReceiver(QObject *parent) : QObject(parent)
{
}

int
QEventReceiver::handle_input( ACE_HANDLE )
{
    return 0;
}

int
QEventReceiver::handle_input( acewrapper::McastHandler& mcast, ACE_HANDLE )
{
    ACE_Message_Block * mb = new ACE_Message_Block( 2000 );
    ACE_Message_Block * pfrom = new ACE_Message_Block( 512 );
    ACE_INET_Addr * pFromAddr = new (pfrom->wr_ptr()) ACE_INET_Addr();

    int res = mcast.recv( mb->wr_ptr(), 2000, *pFromAddr );
    if (res == (-1)) {
		perror("handle_input mcast.recv");
        ACE_Message_Block::release( mb );
        ACE_Message_Block::release( pfrom );
        return 0;
    }
    mb->length( res );
    mb->cont( pfrom );

    emit signal_mcast_input( mb );
    return 0;
}

int
QEventReceiver::handle_timeout( const ACE_Time_Value& tv, const void * )
{
   return 0;
}

int
QEventReceiver::handle_close( ACE_HANDLE, ACE_Reactor_Mask )
{
    return 0;
}
