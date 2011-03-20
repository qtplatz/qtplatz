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

#include "filesystem.h"
#include <boost/interprocess/detail/os_file_functions.hpp>

using namespace adfs;
using namespace adfs::filesystem;

# if defined WIN32
# include "apiwin32.h"
typedef adfs::filesystem::detail::win32api impl;
# else
# include "apiposix.h"
typedef adfs::filesystem::detail::posixapi impl;
# endif

bool
filesystem::grow( const char * path, std::size_t size )
{
    unsigned long long csize = boost::filesystem::file_size( path );
    return impl::resize_file( path, csize + size );
}

bool
filesystem::grow( const boost::filesystem::path& path, std::size_t size )
{
    unsigned long long csize = boost::filesystem::file_size( path );
    return impl::resize_file( path.c_str(), csize + size );
}