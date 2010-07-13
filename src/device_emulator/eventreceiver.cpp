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

QEventReceiver::QEventReceiver(QObject *parent) : QObject(parent)
{
}

int
QEventReceiver::handle_input( ACE_HANDLE )
{
    return 0;
}

int
QEventReceiver::handle_input(acewrapper::DgramHandler& dgram, ACE_HANDLE h)
{
   // fix me
    static char buf[4096];
    static ACE_INET_Addr from;
    int res = dgram.recv( buf, sizeof(buf), from );

    // ACE_Message_Block * mb = new ACE_Message_Block( res + sizeof(ACE_INET_Addr) );
    std::string fromaddr = acewrapper::string( from );
    std::cout << "handle_input dgram (" << h << "," << res << ")" << buf << fromaddr << std::endl;
    emit signal_dgram_input( buf, res, &from );

    return res;
}

int
QEventReceiver::handle_input(acewrapper::McastHandler& mcast, ACE_HANDLE h)
{
    static char buf[4096];
    static ACE_INET_Addr from;

    memset( buf, 0, sizeof(buf) );
    int res = mcast.recv( buf, sizeof(buf), from );
	if (res == (-1)) {
		perror("handle_input mcast.recv");
		res = 0;
	}

	ACE_Message_Block * mb = new ACE_Message_Block( 256 + res );
	do {
		acewrapper::OutputCDR cdr(mb);
		cdr << unsigned short( 0xfffe ); // endian mark
		cdr << unsigned short( 0x0001 ); // protocol version
		cdr << std::string( acewrapper::string( from ) );
        cdr << unsigned long(res);
		cdr.write( buf, res );
        char * wptr = mb->wr_ptr();
        char * rptr = mb->rd_ptr();
		size_t len = mb->length();
		size_t clen = static_cast<ACE_OutputCDR&>(cdr).length();
        mb->length( clen );
	} while (0);

	// char * pdata = mb->rd_ptr();
	std::cout << "handle_input mcast  (" << h << "," << res << ")" << buf << std::endl;
    emit signal_mcast_input( mb );
    return 0;
	// emit signal_mcast_input( buf, res, &from );
	return 0;
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
