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

#if defined __APPLE__

# include <sys/socket.h>
# include <net/if.h> // this must be earlier than ifaddrs.h (see BSD manual, BUGS)
# include <ifaddrs.h>
# include <netinet/in.h>
# include <arpa/inet.h>
//# include <net/if_dl.h>
//# include <net/if_types.h>

#include <string>
#include <vector>

namespace acewrapper {

    namespace macosx {

        class ifconfig {
        public:

            static bool if_addrs( std::vector< std::pair< std::string, std::string > >& addrs ) {

                struct ifaddrs * ifa_list, * ifa;

                if ( getifaddrs( &ifa_list ) < 0 )
                    return false;

                for ( ifa = ifa_list; ifa != 0; ifa = ifa->ifa_next ) {

                    if ( ifa->ifa_addr->sa_family == AF_INET ) {
                        const in_addr& addr = reinterpret_cast< sockaddr_in * >(ifa->ifa_addr)->sin_addr;
                        baddrs.push_back( std::make_pair<std::string, std::string>( ifa->ifa_name, inet_ntoa( aaddr ) ) );
                    }
                }

                freeifaddrs( ifa_list );
                return true;
            }
        
            static bool if_broadaddrs( std::vector< std::pair< std::string, std::string > >& baddrs ) {

                struct ifaddrs * ifa_list, * ifa;

                if ( getifaddrs( &ifa_list ) < 0 )
                    return false;

                for ( ifa = ifa_list; ifa != 0; ifa = ifa->ifa_next ) {

                    if ( ifa->ifa_addr->sa_family == AF_INET ) {

                        if ( ifa->ifa_flags & IFF_BROADCAST ) {
                            in_addr baddr;
                            const in_addr& addr = reinterpret_cast< sockaddr_in * >(ifa->ifa_addr)->sin_addr;
                            const in_addr& mask = reinterpret_cast< sockaddr_in * >(ifa->ifa_netmask)->sin_addr;
                            baddr.s_addr = addr.s_addr | ~mask.s_addr;
                            baddrs.push_back( std::make_pair<std::string, std::string>( ifa->ifa_name, inet_ntoa( baddr ) ) );
                        }
                        if ( ifa->ifa_flags & IFF_POINTOPOINT ) {
                            ; // std::cout << "ifa_name: " << ifa->ifa_name << "\tflags: " << std::hex << ifa->ifa_flags;
                        }
                    }
                }

                freeifaddrs( ifa_list );
                return true;
            }
        };
    } // namespace macosx

}

#endif // defined __APPLE__
