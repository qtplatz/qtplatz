// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#ifndef MCASTHANDLER_H
#define MCASTHANDLER_H

# include <ace/SOCK_Dgram_Mcast.h>

class ACE_INET_Addr;

namespace acewrapper {

   class McastHandler {
   public:
	 McastHandler();
	 ACE_HANDLE get_handle() const;
	 bool open( u_short port = 0 );
	 bool send( const char *, ssize_t );
	 int recv( char * pbuf, ssize_t octets, ACE_INET_Addr& remote_addr );
     bool close();
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
