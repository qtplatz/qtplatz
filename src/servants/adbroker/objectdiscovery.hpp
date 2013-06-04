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

#ifndef OBJECTDISCOVERY_H
#define OBJECTDISCOVERY_H

#include <map>
#include <string>

namespace boost { class mutex; }
namespace acewrapper { class ReactorThread; }

class ACE_INET_Addr;

namespace adbroker {

    class ObjectDiscovery {
    public:
        ~ObjectDiscovery();
        ObjectDiscovery( boost::mutex& mutex );
        bool open();
        void close();
        void operator()( const char *, int, const ACE_INET_Addr& );
        void register_lookup( const std::string& name, const std::string& ident );
        bool unregister_lookup( const std::string& ident, std::string& name );
        int handle_timeout();

    private:
        acewrapper::ReactorThread * reactor_thread_;
        class BcastHandler * bcast_;
		class TimerHandler * timer_;
        bool suspend_;
        boost::mutex& mutex_;
		std::map< std::string, std::string > list_;
    };

}

#endif // OBJECTDISCOVERY_H
