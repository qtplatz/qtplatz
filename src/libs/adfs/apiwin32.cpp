// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "apiwin32.hpp"
#include <windows.h>
#include <boost/filesystem.hpp>

using namespace adfs;
using namespace adfs::detail;

/**
**/

namespace adfs { namespace detail { namespace winapi {

    bool resize_file( HANDLE handle, unsigned long long size ) {
        LARGE_INTEGER sz;
        sz.QuadPart = size;
        return handle != INVALID_HANDLE_VALUE
            && ::SetFilePointerEx(handle, sz, 0, FILE_BEGIN)
            && ::SetEndOfFile(handle)
            && ::CloseHandle(handle);
    }
    //------------------------------------------//
}
}
} // adfs

template<> bool
win32api::resize_file( const char * path, unsigned long long size )
{
    HANDLE handle = CreateFileA( path
                               , GENERIC_WRITE
                               , FILE_SHARE_WRITE | FILE_SHARE_READ
                               , 0
                               , OPEN_EXISTING
                               , FILE_ATTRIBUTE_NORMAL, 0);
    return winapi::resize_file( handle, size );
}

template<> bool
win32api::resize_file( const wchar_t * path, unsigned long long size )
{
    HANDLE handle = CreateFileW( path
                               , GENERIC_WRITE
                               , FILE_SHARE_WRITE | FILE_SHARE_READ
                               , 0
                               , OPEN_EXISTING
                               , FILE_ATTRIBUTE_NORMAL, 0);
    return winapi::resize_file( handle, size );
}

template<> std::string
win32api::get_login_name()
{
    char name[ 1024 ];
    DWORD size = sizeof( name ) / sizeof( name[0] );
    if ( GetUserNameA( name, &size ) )
        return std::string( name );
    return std::string();
}

template<> std::wstring
win32api::get_login_name()
{
    wchar_t name[ 1024 ];
    DWORD size = sizeof( name ) / sizeof( name[0] );
    if ( GetUserNameW( name, &size ) )
        return std::wstring( name );
    return std::wstring();
}

boost::uuids::uuid // std::wstring
win32api::create_uuid()
{
    return boost::uuids::random_generator()();
    // std::wstring guidString;
    // GUID guid;
    // if ( CoCreateGuid( &guid ) == S_OK ) {
    //     LPOLESTR psz;
    //     if ( ::StringFromCLSID( guid, &psz ) == S_OK ) {
    //         guidString = psz;
    //         CoTaskMemFree( psz );
    //     }
    // }
    // return guidString;
}
