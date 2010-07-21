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
QEventReceiver::handle_input(acewrapper::DgramHandler& dgram, ACE_HANDLE /* h */)
{
    ACE_Message_Block * mb = new ACE_Message_Block( 2000 );
    ACE_Message_Block * pfrom = new ACE_Message_Block( 512 );
    ACE_INET_Addr * pFromAddr = new (pfrom->wr_ptr()) ACE_INET_Addr();

    memset( mb->wr_ptr(), mb->size(), 2000 );
	int res = dgram.recv( mb->wr_ptr(), mb->size(), *pFromAddr );
    if (res == (-1)) {
		perror("handle_input dgram.recv");
        ACE_Message_Block::release( mb );
        ACE_Message_Block::release( pfrom );
        return 0;
    }
    mb->length( res );
    mb->cont( pfrom );
    emit signal_dgram_input( mb );
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

#ifdef _DEBUG
    ACE_INET_Addr remote_addr;
    std::ostringstream o;

    do { // packet data serialize/deserialize validation
        using namespace acewrapper;
        using namespace adportable::protocol;

        LifeCycleFrame frame;
        LifeCycleData data;
        lifecycle_frame_serializer::unpack( mb, frame, data );
        try {
            LifeCycle_Hello& hello = boost::get< LifeCycle_Hello& >(data);
            remote_addr.string_to_addr( hello.ipaddr_.c_str() );
            assert( remote_addr.get_port_number() == hello.portnumber_ );
        } catch ( std::bad_cast ) {
            ACE_Message_Block::release(mb);
            return 0;
        }
 
    } while(0);
#endif

    std::cout << "handle_input mcast(" << res << ")" << o.str().c_str() << std::endl;
    emit signal_mcast_input( mb );
    return 0;
}

int
QEventReceiver::handle_timeout( const ACE_Time_Value& tv, const void * )
{
   emit signal_timeout( tv.sec(), tv.usec() );
   return 0;
}

int
QEventReceiver::handle_close( ACE_HANDLE, ACE_Reactor_Mask )
{
    return 0;
}
