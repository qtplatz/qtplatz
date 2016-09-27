/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "document.hpp"
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace adprocessor { class dataprocessor; }

using namespace counting2d;

document::document()
{
}

bool
document::populate( std::shared_ptr< adprocessor::dataprocessor > p )
{
    if ( ( processor_ = p ) ) {

        adfs::stmt sql( *processor_->db() );

        static const auto objuuid_malpix = boost::uuids::string_generator()( "{62ede8f7-dfa3-54c3-a034-e012173e2d10}" );

        sql.prepare( "SELECT COUNT(*) FROM AcquiredConf WHERE objuuid = ?" );
        sql.bind( 1 ) = objuuid_malpix;
        
        while ( sql.step() == adfs::sqlite_row )
            return true;
    }
    return false;
}
