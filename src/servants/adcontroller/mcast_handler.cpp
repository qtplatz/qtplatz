/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "mcast_handler.hpp"
#include <acewrapper/mcasthandler.hpp>
#include <ace/INET_Addr.h>

mcast_handler::mcast_handler(void)
{
}

mcast_handler::~mcast_handler(void)
{
}

int
mcast_handler::handle_input( acewrapper::McastHandler& mcast, ACE_HANDLE )
{
   char rbuf[128];
   ACE_INET_Addr client;
   if ( mcast.recv( rbuf, sizeof(rbuf), client ) ) {
	   if ( strncmp( rbuf, "ior?", 4 ) == 0 )
		   mcast.send( ior_.c_str(), ior_.size() + 1 );
   }
   return 0;
}

int
mcast_handler::handle_timeout( const ACE_Time_Value&, const void * )
{
	return 0;
}

int
mcast_handler::handle_close( ACE_HANDLE, ACE_Reactor_Mask )
{
	return 0;
}
