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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>

namespace adfs {

    boost::uuids::uuid
    create_uuid()
    {
        return boost::uuids::random_generator()();
    }

    template<> ADFSSHARED_EXPORT std::wstring
    to_string( const boost::uuids::uuid & uuid )
    {
        return boost::lexical_cast< std::wstring >( uuid );
    }

    template<> ADFSSHARED_EXPORT std::string
    to_string( const boost::uuids::uuid & uuid )
    {
        return boost::lexical_cast< std::string >( uuid );
    }
}
////////////////////
