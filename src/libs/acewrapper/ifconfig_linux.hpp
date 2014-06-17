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

#if defined __linux__ 

# include <sys/ioctl.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <net/if.h>
# include <vector>

namespace acewrapper {

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

            static bool if_addr( int fd, const std::string& ifname, std::string& addr ) {
                ifreq_impl if_req( ifname );
                if ( ioctl( fd, SIOCGIFADDR, if_req.p() ) == 0 ) {
                    if ( if_req.p()->ifr_broadaddr.sa_family == AF_INET ) {
                        const sockaddr_in * ipv4 = reinterpret_cast< const sockaddr_in *>(&(if_req.p()->ifr_addr));
                        addr = inet_ntoa( ipv4->sin_addr );
                        return true;
                    }
                }
                return false;
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
                return false;
            }

            static bool if_addrs( std::vector< std::pair< std::string, std::string > >& vec ) {
                int fd = socket( PF_INET, SOCK_DGRAM, 0 );
                if ( fd < 0 )
                    return false;
                std::string ifname;
                for ( int idx = 1; if_name( fd, idx, ifname ); ++idx ) {
                    short flags = if_flags( fd, ifname );
                    if ( !( flags & IFF_LOOPBACK ) ) {
                        std::string addr;
                        if ( if_addr( fd, ifname, addr ) ) 
                            vec.push_back( std::pair<std::string, std::string>( ifname, addr ) );
                    }
                }
                close( fd );
                return true;
                
            }

            static bool if_broadaddrs( std::vector< std::pair< std::string, std::string > >& vec ) {
                int fd = socket( PF_INET, SOCK_DGRAM, 0 );
                if ( fd < 0 )
                    return false;
                std::string ifname;
                for ( int idx = 1; if_name( fd, idx, ifname ); ++idx ) {
                    short flags = if_flags( fd, ifname );
                    if ( flags & IFF_BROADCAST && !( flags & IFF_LOOPBACK ) ) {
                        std::string bcast;
                        if ( if_broadaddr( fd, ifname, bcast ) ) 
                            vec.push_back( std::pair<std::string, std::string>( ifname, bcast ) );
                    }
                }
                close( fd );
                return true;
            }
        };
    } // namespace os_linux
}

#endif // linux
