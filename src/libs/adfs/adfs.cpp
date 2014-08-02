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

#include "adfs.hpp"
#include "sqlite.hpp"
#include "sqlite3.h"
#include <iostream>
#include <boost/filesystem.hpp>

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <compiler/diagnostic_pop.h>

#if defined WIN32
# include "apiwin32.hpp"
typedef adfs::detail::win32api impl;
#else
# include "apiposix.hpp"
typedef adfs::detail::posixapi impl;
#endif

std::wstring
adfs::create_uuid()
{
    return impl::create_uuid();
}

const char * 
adfs::null_safe( const char * s )
{
    return ( s ? s : "" );
}
////////////////////
