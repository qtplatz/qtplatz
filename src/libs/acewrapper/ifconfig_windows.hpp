/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#pragma once

#if defined WIN32

// # include <winsock2.h>
// # include <ws2tcpip.h>
// # include <iphlpapi.h>
// # pragma comment( lib, "iphlpapi.lib" )
// # pragma comment( lib, "ws2_32.lib" )

// # include <sys/socket.h>
// # include <net/if.h> // this must be earlier than ifaddrs.h (see BSD manual, BUGS)
// # include <ifaddrs.h>
// # include <netinet/in.h>
// # include <arpa/inet.h>
// //# include <net/if_dl.h>
// //# include <net/if_types.h>

#include <string>
#include <vector>

namespace acewrapper {

    namespace windows {

        class ifconfig {
        public:
            static bool if_addrs( std::vector< std::pair< std::string, std::string > >& addrs );
            static bool if_broadaddrs( std::vector< std::pair< std::string, std::string > >& baddrs );
        };
    }
}

#endif // defined WIN32
