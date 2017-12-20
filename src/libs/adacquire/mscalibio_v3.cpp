/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mscalibio_v3.hpp"
#include <adfs/sqlite.hpp>

using namespace adacquire::v3;

mscalibio::mscalibio()
{
}


// static, v3 api
bool
mscalibio::create_table_v3( adfs::sqlite& db )
{
    adfs::stmt sql( db );
    if ( sql.exec(
             "CREATE TABLE  \
 Calibration (              \
 objuuid      UUID          \
,calibId      TEXT          \
,dataClass    TEXT          \
,data         BLOB          \
,revision     INTEGER PRIMARY KEY AUTOINCREMENT \
)" ) ) 
		return true;
	return false;
}

// static, v3 api
bool
mscalibio::readCalibration( adfs::sqlite& db
                            , const boost::uuids::uuid& objUuid
                            , const std::string& dataClass
                            , std::vector< char >& device
                            , int64_t& revision )
{
    adfs::stmt sql( db );

    if ( sql.prepare( "SELECT rowid,revision FROM Calibration WHERE dataClass =? AND objuuid=? ORDER BY revision desc" ) ) {

        sql.bind( 1 ) = dataClass;
        sql.bind( 2 ) = objUuid;

        if ( sql.step() == adfs::sqlite_row ) {
            
            uint64_t rowid = sql.get_column_value< int64_t >( 0 );
            revision = sql.get_column_value< int64_t >( 1 );
            adfs::blob blob;
            
            if ( blob.open( db, "main", "Calibration", "data", rowid, adfs::readonly ) ) {
                device.resize( blob.size() );
                if ( blob.read( reinterpret_cast< int8_t *>( device.data() ), device.size() ) )
                    return true;
            }
        }
    }
    return false;
}

// v3
bool
mscalibio::writeCalibration( adfs::sqlite& db
                             , const boost::uuids::uuid& objUuid
                             , const std::string& calibId
                             , const std::string& dataClass
                             , const char * data, size_t size )
{
    adfs::stmt sql( db );

    if ( sql.prepare( "SELECT name FROM sqlite_master WHERE type='table' AND name = 'Calibration'" ) ) {
        if ( sql.step() != adfs::sqlite_done )
            if ( ! create_table_v3( db ) )
                return false;
    }
    sql.reset();

    const char * query = "INSERT INTO Calibration (objuuid,calibId,dataClass,data) VALUES(:objuuid,:calibId,:dataClass,:data)";
    
    if ( sql.prepare( query ) ) {
    
        sql.bind( 1 ) = objUuid;
        sql.bind( 2 ) = calibId;
        sql.bind( 3 ) = dataClass;
        sql.bind( 4 ) = adfs::blob( size, reinterpret_cast< const int8_t *>( data ) );
        
        if ( sql.step() == adfs::sqlite_done )
            return true;
    }

    sql.reset();
    return false;
}
