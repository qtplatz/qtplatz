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

#include "ifconfig.hpp"

#include <adportable/debug.hpp>
#include <boost/bind.hpp>

#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <algorithm>

#if defined __APPLE__
# include "ifconfig_macosx.hpp"
#elif defined __linux__
# include "ifconfig_linux.hpp"
#elif defined WIN32
# include "ifconfig_windows.hpp"
#endif

using namespace acewrapper;

ifconfig::ifconfig()
{
}

bool
ifconfig::if_addrs(  std::vector< std::pair< std::string, std::string > >& vec )
{
#if defined __linux__
    return acewrapper::os_linux::ifconfig::if_addrs( vec );
#endif

#if defined __APPLE__
    return acewrapper::macosx::ifconfig::if_addrs( vec );
#endif

#if defined WIN32
    return acewrapper::windows::ifconfig::if_addrs( vec );
#endif

}

bool
ifconfig::broadaddr( std::vector< std::pair< std::string, std::string > >& vec )
{
    vec.clear();

#if defined __linux__
    return acewrapper::os_linux::ifconfig::if_broadaddrs( vec );
#endif

#if defined __APPLE__
    return acewrapper::macosx::ifconfig::if_broadaddrs( vec );
#endif

#if defined WIN32
    return acewrapper::windows::ifconfig::if_broadaddrs( vec );
#endif

    return false;
}

