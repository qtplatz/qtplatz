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

#include "ifconfig_windows.hpp"

#if defined WIN32

# include <winsock2.h>
# include <ws2tcpip.h>
# include <iphlpapi.h>
# pragma comment( lib, "iphlpapi.lib" )
# pragma comment( lib, "ws2_32.lib" )

#include <algorithm>
#include <memory>
#include <sstream>
#include <boost/format.hpp>

using namespace acewrapper::windows;

bool
ifconfig::if_broadaddrs( std::vector< std::pair< std::string, std::string > >& vec )
{
	PMIB_IPADDRTABLE pIPAddrTable = 0;
	DWORD dwSize = 0;

    std::unique_ptr< char[] > pbuf;
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
                    std::string bcast = inet_ntoa( in_bcast );
                    if ( std::find_if( vec.begin(), vec.end(), [&] ( const std::pair< std::string, std::string>& a ){ return a.second == bcast; } ) == vec.end() )
                        vec.push_back( std::make_pair( (boost::format( "%d" ) % i).str(), bcast ) );
				}
			}
		}
	}
    return !vec.empty();
}

bool
ifconfig::if_addrs( std::vector< std::pair< std::string, std::string > >& vec )
{
	PMIB_IPADDRTABLE pIPAddrTable = 0;
	DWORD dwSize = 0;

    std::unique_ptr< char[] > pbuf;
	if ( GetIpAddrTable( 0, &dwSize, 0 ) == ERROR_INSUFFICIENT_BUFFER ) {
		pbuf.reset( new char [ dwSize ] );
		pIPAddrTable = reinterpret_cast< MIB_IPADDRTABLE * >( pbuf.get() );
	}
	DWORD dwRetVal;
	if ( ( dwRetVal = GetIpAddrTable( pIPAddrTable, &dwSize, 0 ) ) == NO_ERROR ) {
		for ( size_t i = 0; i < pIPAddrTable->dwNumEntries; ++i ) {
			if ( ! ( pIPAddrTable->table[ i ].wType & ( MIB_IPADDR_DISCONNECTED | MIB_IPADDR_DELETED | MIB_IPADDR_TRANSIENT ) ) ) {
                IN_ADDR in_addr;
				in_addr.S_un.S_addr = pIPAddrTable->table[ i ].dwAddr;
                std::string addr = inet_ntoa( in_addr );
                if ( addr != "127.0.0.1" )
                    vec.push_back( std::make_pair( (boost::format( "%d" ) % i).str(), addr ) );
			}
		}
        return true;
	}
    return false;
}

#endif
