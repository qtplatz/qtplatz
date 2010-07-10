// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef MCASTSERVER_H
#define MCASTSERVER_H

#include <ace/SOCK_Dgram_Mcast.h>
#include <ace/Event_Handler.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <boost/utility.hpp>
#include "callback.h"

class McastServer : boost::noncopyable {
public:
	~McastServer();
    McastServer( acewrapper::Callback& cb, ACE_Reactor * r = 0, u_short port = 0);
    // static void * thread_entry( void * me );

    ACE_Reactor * get_reactor();
	bool send( const char * pbuf, ssize_t nsize, const ACE_INET_Addr& );
	bool send( const char * pbuf, ssize_t nsize );

private:
    class Handler : public ACE_Event_Handler {
        acewrapper::Callback& callback_;
        ACE_SOCK_Dgram_Mcast mcast_;
        ACE_INET_Addr sock_addr_;
        public:
        ~Handler();
        Handler(u_short udp_port, const char * ip_addr, ACE_Reactor&, acewrapper::Callback& );
        // demuxer hooks
        virtual int handle_input(ACE_HANDLE);
        virtual int handle_close(ACE_HANDLE, ACE_Reactor_Mask);
        virtual ACE_HANDLE get_handle() const;

        //
        bool send( const char * pbuf, ssize_t size );
    };

private:
    ACE_Reactor * reactor_;
    ACE_Recursive_Thread_Mutex mutex_;
    u_short port_;
    Handler * handler_;
    acewrapper::Callback& callback_;

};

#endif // MCASTSERVER_H
