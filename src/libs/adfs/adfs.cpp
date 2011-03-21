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

#include "adfs.h"
#include "sqlite3.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "adsqlite.h"

using namespace adfs;

namespace adfs { namespace detail {
  /*
  */
}
}

//////
storage::storage() : db_(0)
{
}

storage::storage( const storage& t ) : db_( t.db_ )
{
}

storage::~storage()
{
    delete db_;
}

bool
storage::create( const char * filename )
{
    boost::filesystem::path filepath( filename );

    boost::filesystem::remove( filepath );

    db_ = new sqlite();
    return db_->create( filepath.c_str() );
}

bool
storage::close()
{
    return db_ && db_->close();
}

////////////////////
////////////////////
