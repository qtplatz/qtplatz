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

#ifndef OBJECTDISCOVERY_H
#define OBJECTDISCOVERY_H

//#include <ace/Reactor.h>
// #include <ace/Singleton.h>
#include <map>
#include <string>

class ACE_INET_Addr;
class ACE_Recursive_Thread_Mutex;

namespace acewrapper { class ReactorThread; }

namespace adbroker {

    class ObjectDiscovery {
    public:
        ~ObjectDiscovery();
        ObjectDiscovery( ACE_Recursive_Thread_Mutex& mutex );
        // void event_loop();
        // static void * thread_entry( void * );
        bool open();
        void close();
        void operator()( const char *, int, const ACE_INET_Addr& );
        void register_lookup( const std::string& name, const std::string& ident );
        bool unregister_lookup( const std::string& ident, std::string& name );
        int handle_timeout();
        // inline ACE_Reactor * reactor() { return reactor_; }

    private:
        // ACE_thread_t t_handle_;
        // ACE_Reactor * reactor_;
        // class McastHandler * mcast_;
        acewrapper::ReactorThread * reactor_thread_;
        class BcastHandler * bcast_;
        bool suspend_;
        ACE_Recursive_Thread_Mutex& mutex_;
        std::map< std::string, std::string> list_;
    };

}

#endif // OBJECTDISCOVERY_H
