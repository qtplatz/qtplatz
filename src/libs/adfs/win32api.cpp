// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "win32api.h"
#include <windows.h>
#include <boost/filesystem.hpp>

using namespace adfs;
using namespace adfs::filesystem;
using namespace adfs::filesystem::detail;

/**
**/

namespace adfs { namespace filesystem { namespace detail { namespace winapi {

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
