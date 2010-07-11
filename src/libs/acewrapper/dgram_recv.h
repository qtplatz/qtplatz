// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DGRAM_RECV_H
#define DGRAM_RECV_H

class ACE_INET_Addr;

namespace acewrapper {

   template<class T> struct dgram_recv {
	 T& t_;
	 dgram_recv(T& t) : t_(t) {}
	 virtual int operator()( char * pbuf, int bufsize, ACE_INET_Addr& from ) { t_.recv(pbuf, bufsize, from); }
   };
}

#endif // DGRAM_RECV_H
