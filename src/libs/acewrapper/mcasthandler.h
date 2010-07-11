// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef MCASTHANDLER_H
#define MCASTHANDLER_H

#include <ace/SOCK_Dgram_Mcast.h>

class ACE_INET_Addr;

namespace acewrapper {

   class McastHandler {
      public:
	 McastHandler();
	 ACE_HANDLE get_handle() const;
	 bool open( u_short port = 0 );
	 bool send( const char *, ssize_t );
	 int recv( char * pbuf, ssize_t octets, ACE_INET_Addr& remote_addr );
      private:
	 ACE_SOCK_Dgram_Mcast sock_mcast_;
	 ACE_INET_Addr sock_addr_;
   };
   
   template<class T> class McastReceiver : public T
					 , public McastHandler {
      public:
	 McastReceiver() {}
	 int handle_input( ACE_HANDLE h ) {  return T::handle_input(*this, h); }
	 // int handle_timeout( const ACE_Time_Value& tv, const void * arg) { return T::handle_timeout(tv, arg); }
	 // int handle_close( ACE_HANDLE h, ACE_Reactor_Mask mask ) { return T::handle_close( h, mask ); }
   };

}

#endif // MCASTHANDLER_H
