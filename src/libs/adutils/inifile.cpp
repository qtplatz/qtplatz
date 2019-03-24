/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "inifile.hpp"
#include <adfs/sqlite.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adportable/serializer.hpp>
#include <adportable/debug.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace adutils;

bool
inifile::save( adfs::sqlite& db, const adcontrols::ControlMethod::Method& cm )
{
    adfs::stmt sql( db );

#if !defined NDEBUG && 0
    ADDEBUG() << "************ inifile::save *********";
    for ( auto& mi : cm )
        ADDEBUG() << mi.clsid() << ", " << mi.modelname() << ", " << mi.itemLabel();
    ADDEBUG() << "************************************";
#endif

    // check adutils/AcquiredData::create_table_v3
    sql.exec( "CREATE TABLE IF NOT EXISTS MetaData ( clsid UUID, attrib TEXT, data BLOB )" );
    std::string ar;
    if ( adportable::bin_serializer()( cm, ar ) ) {

        sql.prepare( "INSERT OR REPLACE INTO MetaData ( clsid, attrib, data ) VALUES ( ?,?,? )" );
        sql.bind( 1 ) = adcontrols::ControlMethod::Method::clsid();
        sql.bind( 2 ) = std::string( "ControlMethod::Method" );
        sql.bind( 3 ) = adfs::blob( ar.size(), reinterpret_cast< const int8_t * >( ar.data() ) );

        return sql.step() == adfs::sqlite_done;
    }

    return false;
}

bool
inifile::load( adfs::sqlite& db, adcontrols::ControlMethod::Method& cm )
{
    adfs::stmt sql( db );

    // check adutils/AcquiredData::create_table_v3
    sql.prepare( "SELECT data FROM MetaData WHERE clsid = ?" );
    sql.bind( 1 ) = adcontrols::ControlMethod::Method::clsid();

    if ( sql.step() == adfs::sqlite_row ) {
        auto blob = sql.get_column_value< adfs::blob >( 0 );
        if ( adportable::bin_deserializer()( cm, reinterpret_cast< const char *>( blob.data() ), blob.size() ) ) {
#if !defined NDEBUG && 0
            ADDEBUG() << "************ inifile::load *********";
            for ( auto& mi : cm )
                ADDEBUG() << mi.clsid() << ", " << mi.modelname() << ", " << mi.itemLabel();
            ADDEBUG() << "************************************";
#endif
            return true;
        }

    }

    return false;
}
