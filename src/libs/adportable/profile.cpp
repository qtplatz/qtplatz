/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "./profile.hpp"
#include "./string.hpp"

#if defined WIN32
# include <shlobj.h> // see ShGetFolderLocation API
#else
# include <pwd.h>
#endif

namespace adportable {

    profile::profile()
    {
    }
    
    template<> std::string
    profile::user_login_name()
    {
#if WIN32
        
#else
        struct passwd * pw = getpwuid( geteuid() );
        return pw->pw_name;
#endif
    }
    
    template<> std::wstring
    profile::user_login_name()
    {
#if WIN32
        
#else
        struct passwd * pw = getpwuid( geteuid() );
        return adportable::string::convert( pw->pw_name );
#endif
    }
    
    template<> std::string
    profile::user_data_dir()
    {
#if WIN32
        
#else
        struct passwd * pw = getpwuid( geteuid() );
        return pw->pw_dir;
#endif
    }

    template<> std::wstring
    profile::user_data_dir()
    {
#if WIN32
        
#else
        struct passwd * pw = getpwuid( geteuid() );
        return adportable::string::convert( pw->pw_dir );
#endif
    }
}

