/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#include "./profile.hpp"
#include "./string.hpp"

#if defined WIN32
# define SECURITY_WIN32 1
# include <Windows.h>
# include <security.h>
# include <shlobj.h> // see ShGetFolderLocation API
namespace adportable { namespace detail { struct winapi; } }
typedef adportable::detail::winapi impl;
#else
# include <pwd.h>
# include <unistd.h>
# include <sys/types.h>
namespace adportable { namespace detail { struct posixapi; } }
typedef adportable::detail::posixapi impl;
#endif

namespace adportable { namespace detail {

#if defined WIN32
	struct winapi {
		template<class char_type> static std::basic_string<char_type> user_data_dir();
        template<class char_type> static std::basic_string<char_type> user_login_name();
        template<class char_type> static std::basic_string<char_type> user_login_id();
        template<class char_type> static std::basic_string<char_type> computer_name();

        template<typename char_type> static std::basic_string<char_type> user_login_name_( EXTENDED_NAME_FORMAT format ) {

            typedef BOOLEAN( WINAPI *api_type )(EXTENDED_NAME_FORMAT, char_type*, PULONG);

            if ( auto hModule = LoadLibraryA( "Secur32.dll" ) ) {
                const char * entry_point = (sizeof( char_type ) == 1) ? "GetUserNameExA" : "GetUserNameExW";
                if ( auto api = reinterpret_cast<api_type>(GetProcAddress( hModule, entry_point )) ) {
                    char_type path[ MAX_PATH ];
                    DWORD size = sizeof( path ) / sizeof( char );
                    if ( api( format, path, &size ) )
                        return path;
                }
                FreeLibrary( hModule );
            }
            return std::basic_string<char_type>(); // return empty by means of error
        }
	};
	template<> std::string winapi::user_data_dir()
	{
		char path[ MAX_PATH ];
		HRESULT hr
			= SHGetFolderPathA( 0, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, path );
		if ( hr == S_OK )
			return path;
		return std::string(); // return empty by means of error
	}
	template<> std::wstring winapi::user_data_dir()
	{
		wchar_t path[ MAX_PATH ];
		HRESULT hr
			= SHGetFolderPathW( 0, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, path );
		if ( hr == S_OK )
			return path;
		return std::wstring(); // return empty by means of error
	}
    template<> std::string winapi::user_login_name()
	{
        return user_login_name_<char>( NameDisplay );
	}
    template<> std::wstring winapi::user_login_name()
	{
        return user_login_name_<wchar_t>( NameDisplay );
    }

    template<> std::string winapi::user_login_id()
	{
        return user_login_name_<char>( NameCanonical );
	}
    template<> std::wstring winapi::user_login_id()
	{
        return user_login_name_<wchar_t>( NameCanonical );
    }

    template<> std::string winapi::computer_name()
	{
		char name[ MAX_PATH ];
        DWORD size = sizeof( name ) / sizeof( name[0] );
        if ( GetComputerNameExA( ComputerNamePhysicalDnsFullyQualified, name, &size ) )
            return name;
        return std::string();
	}
    template<> std::wstring winapi::computer_name()
	{
		wchar_t name[ MAX_PATH ];
        DWORD size = sizeof( name ) / sizeof( name[0] );
        if ( GetComputerNameExW( ComputerNamePhysicalDnsFullyQualified, name, &size ) )
            return name;
        return std::wstring();
    }

#else
	struct posixapi {
		template<class char_type> static std::basic_string<char_type> user_data_dir();
	};

    template<> std::string posixapi::user_data_dir()
	{
		struct passwd * pw = getpwuid( geteuid() );
        return pw->pw_dir;
	}

    template<> std::wstring posixapi::user_data_dir()
	{
		struct passwd * pw = getpwuid( geteuid() );
        return adportable::string::convert( pw->pw_dir );
	}

    template<> std::string posixapi::user_login_name()
	{
		struct passwd * pw = getpwuid( geteuid() );
        return pw->pw_gecos;
	}

    template<> std::wstring posixapi::user_login_name()
	{
		struct passwd * pw = getpwuid( geteuid() );
        return adportable::string::convert( pw->pw_gecos );
	}

    template<> std::string posixapi::user_login_id()
	{
		struct passwd * pw = getpwuid( geteuid() );
        return pw->pw_name;
	}

    template<> std::wstring posixapi::user_login_id()
	{
		struct passwd * pw = getpwuid( geteuid() );
        return adportable::string::convert( pw->pw_name );
	}

    template<> std::string posixapi::computer_name()
	{
        char hostname[1024];
		gethostname( hostname, sizeof(hostname));
        return return hostname;
	}

    template<> std::wstring posixapi::computer_name()
    {
        char hostname[1024];
		gethostname( hostname, sizeof(hostname));
        return adportable::string::convert( hostname );
	}

#endif

}
}

namespace adportable {

    profile::profile()
    {
    }
    
    template<> std::string
    profile::user_data_dir()
    {
		return impl::user_data_dir<char>();
    }

    template<> std::wstring
    profile::user_data_dir()
    {
		return impl::user_data_dir<wchar_t>();
    }

    template<> std::string
    profile::user_login_name()
    {
		return impl::user_login_name<char>();
    }

    template<> std::wstring
    profile::user_login_name()
    {
		return impl::user_login_name<wchar_t>();
    }

    template<> std::string
    profile::user_login_id()
    {
		return impl::user_login_id<char>();
    }

    template<> std::wstring
    profile::user_login_id()
    {
		return impl::user_login_id<wchar_t>();
    }

    template<> std::string
    profile::computer_name()
    {
		return impl::computer_name<char>();
    }

    template<> std::wstring
    profile::computer_name()
    {
		return impl::computer_name<wchar_t>();
    }

}

