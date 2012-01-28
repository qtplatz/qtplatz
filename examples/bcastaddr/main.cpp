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

#include <QtCore/QCoreApplication>

#include <iostream>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <string>

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

    static bool if_flags( int fd, const std::string& ifname, short& flags ) {
        ifreq_impl if_req( ifname );
        if ( ioctl( fd, SIOCGIFFLAGS, if_req.p() ) < 0 )
            return false;
        flags = if_req.p()->ifr_flags;
        return true;
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

int
main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    int fd = socket( PF_INET, SOCK_DGRAM, 0 );
    if ( fd < 0 ) {
        std::cerr << "error: opening socket" << std::endl;
        return 0;
    }

    std::string name;
    for ( int idx = 1; ifconfig::if_name( fd, idx, name ); ++idx ) {
        short flags;
        if ( ifconfig::if_flags( fd, name, flags ) ) {
            std::cout << name << ", flags=" << std::hex << std::showbase << flags << " ";
            if ( flags & IFF_UP )
                std::cout << "UP ";
            if ( flags & IFF_BROADCAST )
                std::cout << "BROADCAST ";
            if ( flags & IFF_DEBUG )
                std::cout << "DEBUG ";
            if ( flags & IFF_LOOPBACK )
                std::cout << "LOOPBACK ";
            if ( flags & IFF_POINTOPOINT )
                std::cout << "POINTTOPOINT ";
            if ( flags & IFF_RUNNING )
                std::cout << "RUNNING ";
            if ( flags & IFF_NOARP )
                std::cout << "NOARP ";
            if ( flags & IFF_PROMISC )
                std::cout << "PROMISC ";
            if ( flags & IFF_NOTRAILERS )
                std::cout << "NOTRAILERS ";
            if ( flags & IFF_ALLMULTI )
                std::cout << "ALLMULTI ";
            if ( flags & IFF_MASTER )
                std::cout << "MASTER ";
            if ( flags & IFF_SLAVE )
                std::cout << "SLAVE ";
            if ( flags & IFF_MULTICAST )
                std::cout << "MULTICAST ";
            if ( flags & IFF_PORTSEL )
                std::cout << "PORTSEL ";
            if ( flags & IFF_AUTOMEDIA )
                std::cout << "AUTOMEDIA ";
            if ( flags & IFF_DYNAMIC )
                std::cout << "DYNAMIC ";
            // if ( flags & IFF_LOWER_UP )
            //     std::cout << "LOWER_UP ";
            // if ( flags & IFF_DORMANT )
            //     std::cout << "DORMANT ";
            // if ( flags & IFF_ECHO )
            //     std::cout << "ECHO ";
            if ( flags & IFF_BROADCAST ) {
                std::string baddr;
                if ( ifconfig::if_broadaddr( fd, name, baddr ) )
                    std::cout << " BCAST:" << baddr;
            }
            std::cout << std::endl;
        }
    }
}





