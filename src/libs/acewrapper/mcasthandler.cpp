// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "mcasthandler.hpp"
#include <ace/SOCK_Dgram_Mcast.h>
#include <ace/INET_Addr.h>
#include <ace/Default_Constants.h>

using namespace acewrapper;

McastHandler::McastHandler()
{
   sock_addr_ = ACE_INET_Addr( ACE_DEFAULT_MULTICAST_PORT, ACE_DEFAULT_MULTICAST_ADDR );
}

ACE_HANDLE
McastHandler::get_handle() const
{
   return sock_mcast_.get_handle();
}

bool
McastHandler::open( u_short port )
{
    if ( port )
        sock_addr_.set_port_number(port);

    if ( sock_mcast_.join( sock_addr_ ) == (-1) )
        return false;
    return true;
}

bool
McastHandler::send( const char * pbuf, ssize_t size )
{
    ssize_t r = sock_mcast_.send( pbuf, size );
    return r == size;
}

int
McastHandler::recv( char * pbuf, ssize_t bufsize, ACE_INET_Addr& remote_addr)
{
   return sock_mcast_.recv( pbuf, bufsize, remote_addr );
}

bool
McastHandler::close()
{
    return sock_mcast_.close() != (-1);
}
