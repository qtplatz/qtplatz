/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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
#include <boost/foreach.hpp>

#if defined __APPLE__
# include "../../src/libs/acewrapper/ifconfig_macosx.hpp"
#elif defined __linux__
# include "../../src/libs/acewrapper/ifconfig_linux.hpp"
#endif

int
main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    typedef std::pair< std::string, std::string > name_addr_pair_t;
    std::vector< name_addr_pair_t > baddrs;
    bool res;
#if defined __APPLE__
    res = acewrapper::macosx::ifconfig::if_broadaddrs( baddrs );
#else
    res = acewrapper::os_linux::ifconfig::if_broadaddrs( baddrs );
#endif
    if ( res ) {
        BOOST_FOREACH( const name_addr_pair_t& addr, baddrs )
            std::cout << "baddrs: " << addr.first << "\t" << addr.second << std::endl;
    }
    std::cout << "----------------" << std::endl;
}





