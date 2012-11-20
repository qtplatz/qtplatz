// This is a -*- C++ -*- header.
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

#ifndef MCASTSERVER_H
#define MCASTSERVER_H

#  include <ace/SOCK_Dgram_Mcast.h>
#  include <ace/Event_Handler.h>
#  include <ace/Recursive_Thread_Mutex.h>

#include <boost/utility.hpp>
#include "callback.hpp"

class McastServer : boost::noncopyable {
public:
    ~McastServer();
    McastServer( acewrapper::Callback& cb, ACE_Reactor * r = 0, u_short port = 0);

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
