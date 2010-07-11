// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DGRAMHANDLER_H
#define DGRAMHANDLER_H

#include <ace/SOCK_Dgram.h>
#include <ace/Event_Handler.h>

class ACE_INET_Addr;

namespace acewrapper {

   class DgramHandler {
      public:
	 DgramHandler();
	 ACE_HANDLE get_handle() const;
	 
	 bool open( u_short port = 0 );
	 bool send( const char *, ssize_t, const ACE_INET_Addr& );
	 int recv( char * pbuf, int octets, ACE_INET_Addr& remote_addr);

      private:
	 int errno_;
	 ACE_SOCK_Dgram sock_dgram_;
	 ACE_INET_Addr sock_addr_;
   };

   ////////////////
   template<class T> class DgramReceiver : public T
					 , public DgramHandler {
      public:
	 DgramReceiver() {}
	 int handle_input( ACE_HANDLE h ) { return T::handle_input(*this, h); }
	 //int handle_timeout( const ACE_Time_Value& tv, const void * arg) { return T::handle_timeout(tv, arg); }
	 //int handle_close( ACE_HANDLE h, ACE_Reactor_Mask mask ) { return T::handle_close( h, mask ); }
  };

}

#endif // DGRAMHANDLER_H
