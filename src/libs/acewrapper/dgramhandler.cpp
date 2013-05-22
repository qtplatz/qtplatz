// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "dgramhandler.hpp"
#include "callback.hpp"
#include <assert.h>
#include <ace/Reactor.h>

using namespace acewrapper;

static Callback __callback__;

DgramHandler::DgramHandler()
{
}


ACE_HANDLE
DgramHandler::get_handle() const
{
   return sock_dgram_.get_handle();
}

//
bool
DgramHandler::send( const char * pbuf, ssize_t octets, const ACE_INET_Addr& to )
{
    return sock_dgram_.send(pbuf, octets, const_cast<ACE_INET_Addr&>(to)) == octets;
}

int
DgramHandler::recv( char * pbuf, int bufsize, ACE_INET_Addr& remote_addr )
{
    return sock_dgram_.recv(pbuf, bufsize, remote_addr);
}

bool
DgramHandler::open( u_short port )
{
    if ( port )
        sock_addr_ = ACE_INET_Addr( port, "0.0.0.0" );
   
    if ( sock_dgram_.open( sock_addr_ ) == (-1) )
        return false;

    return true;
}

bool
DgramHandler::close()
{
    return sock_dgram_.close() != (-1);
}