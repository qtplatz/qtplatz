/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#include "ifconfig.hpp"

#if defined __linux__
# include <sys/ioctl.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <net/if.h>
#endif

#if defined WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
# include <iphlpapi.h>
# pragma comment( lib, "iphlpapi.lib" )
# pragma comment( lib, "ws2_32.lib" )
#endif

#include <adportable/debug.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>

#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>

namespace acewrapper {
#if defined __linux__
    namespace os_linux {

        class ifconfig {
            class ifreq_impl {
                ifreq if_req;
                inline void clear() { memset( &if_req, 0, sizeof(ifreq) ); }
            public:
                ifreq_impl( int idx ) { clear(); if_req.ifr_ifindex = idx; }
                ifreq_impl( const std::string& ifname ) { 
                    clear(); 
                    strncpy( if_req.ifr_name, ifname.c_str(), IFNAMSIZ );            
                }
                inline ifreq * p() { return &if_req; }
            };
        
        public:
        
            static bool if_name( int fd, int idx, std::string& name ) {
                ifreq_impl if_req( idx );
                if ( ioctl( fd, SIOCGIFNAME, if_req.p() ) < 0 )
                    return false;
                name = if_req.p()->ifr_name;
                return true;
            }
        
            static short if_flags( int fd, const std::string& ifname ) {
                ifreq_impl if_req( ifname );
                if ( ioctl( fd, SIOCGIFFLAGS, if_req.p() ) < 0 )
                    return 0;
                return if_req.p()->ifr_flags;
            }
        
            static bool if_broadaddr( int fd, const std::string& ifname, std::string& baddr ) {
                ifreq_impl if_req( ifname );
                if ( ioctl( fd, SIOCGIFBRDADDR, if_req.p() ) == 0 ) {
                    if ( if_req.p()->ifr_broadaddr.sa_family == AF_INET ) {
                        const sockaddr_in * ipv4 = reinterpret_cast< const sockaddr_in *>(&(if_req.p()->ifr_broadaddr));
                        const in_addr& addr = ipv4->sin_addr;
                        baddr = inet_ntoa( addr );
                        return true;
                    }
                }
                return 0;
            }
        };
    } // namespace linux
#endif
}

using namespace acewrapper;

ifconfig::ifconfig()
{
}

bool
ifconfig::broadaddr( std::vector< std::pair< std::string, std::string > >& vec )
{
    vec.clear();

#if defined __linux__
    int fd = socket( PF_INET, SOCK_DGRAM, 0 );
    if ( fd < 0 ) {
        adportable::debug( __FILE__, __LINE__ ) << "socket open failed.";
        return false;
    }

    using acewrapper::os_linux::ifconfig;

    std::string ifname;
    for ( int idx = 1; ifconfig::if_name( fd, idx, ifname ); ++idx ) {
        short flags = ifconfig::if_flags( fd, ifname );
        if ( flags & IFF_BROADCAST && !( flags & IFF_LOOPBACK ) ) {
            std::string bcast;
            if ( ifconfig::if_broadaddr( fd, ifname, bcast ) ) 
                vec.push_back( std::make_pair<std::string, std::string>( ifname, bcast ) );
        }
    }
    close( fd );
#endif

#if defined WIN32
	PMIB_IPADDRTABLE pIPAddrTable = 0;
	DWORD dwSize = 0;
	boost::scoped_array< char > pbuf;
	if ( GetIpAddrTable( 0, &dwSize, 0 ) == ERROR_INSUFFICIENT_BUFFER ) {
		pbuf.reset( new char [ dwSize ] );
		pIPAddrTable = reinterpret_cast< MIB_IPADDRTABLE * >( pbuf.get() );
	}
	DWORD dwRetVal;
	if ( ( dwRetVal = GetIpAddrTable( pIPAddrTable, &dwSize, 0 ) ) == NO_ERROR ) {
		for ( size_t i = 0; i < pIPAddrTable->dwNumEntries; ++i ) {
			if ( ! ( pIPAddrTable->table[ i ].wType & ( MIB_IPADDR_DISCONNECTED | MIB_IPADDR_DELETED | MIB_IPADDR_TRANSIENT ) ) ) {
   				IN_ADDR in_addr, in_bcast;
				in_addr.S_un.S_addr = pIPAddrTable->table[ i ].dwAddr;
				in_bcast.S_un.S_addr = pIPAddrTable->table[ i ].dwAddr | ~pIPAddrTable->table[ i ].dwMask;
				std::string addr = inet_ntoa( in_addr );
				if ( addr != "127.0.0.1" ) {
					std::ostringstream o;
					o << i;
					// std::string mask = inet_ntoa( in_mask );
					std::string bcast = inet_ntoa( in_bcast );
					if ( std::find_if( vec.begin(), vec.end(), boost::bind( &ifaddr::second, _1) == bcast ) == vec.end() )
						vec.push_back( std::make_pair( o.str(), bcast ) );
					// std::cout << "bcast: " << bcast << " mask: " << mask << std::endl;
				}
			}
		}
	}
    //-------
	return !vec.empty();

#endif
    return ! vec.empty();
}

