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

#include "acquiredconf.hpp"
#include <adcontrols/lockmass.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <typeinfo>

using namespace adutils;

AcquiredConf::AcquiredConf()
{
}

// static
bool
AcquiredConf::create_table( adfs::sqlite& db )
{
    adfs::stmt sql( db );
    if ( sql.exec(
             "CREATE TABLE AcquiredConf (\
 objid                INTEGER       \
,pobjid               INTEGER       \
,uuid                 UUID          \
,dataInterpreterClsid TEXT          \
,trace_method         INTEGER       \
,spectrometer         INTEGER       \
,trace_id             TEXT          \
,trace_display_name   TEXT          \
,axis_x_label         TEXT          \
,axis_y_label         TEXT          \
,axis_x_decimals      INTEGER       \
,axis_y_decimals      INTEGER       \
,UNIQUE(objid)                      \
)" 
             ) )
        return true;
    
    return false;
}

// static
bool
AcquiredConf::create_mslock( adfs::sqlite& db )
{
    adfs::stmt sql( db );
    if ( sql.exec(
             "CREATE TABLE IF NOT EXISTS MSLock ("
             " objid       UUID"
             ",fcn         INTEGER"
             ",rowid       INTEGER"
             ",formula     TEXT"
             ",exactMass   REAL"
             ",matchedMass REAL"
             ",matchedTime REAL)"
             ) )
        return true;
    
    return false;
}

bool
AcquiredConf::delete_mslock( adfs::sqlite& db, const boost::uuids::uuid& objid, int fcn )
{
    adfs::stmt sql( db );
    sql.prepare( "DELETE FROM MSLock WHERE objid = ? AND fcn = ?" );
    sql.bind( 1 ) = objid;
    sql.bind( 2 ) = fcn;
    return sql.step() == adfs::sqlite_done;
}

bool
AcquiredConf::insert( adfs::sqlite& db, const boost::uuids::uuid& objid, int fcn, int64_t rowid, const adcontrols::lockmass::mslock& lkms )
{
    adfs::stmt sql( db );
    sql.begin();
    for ( const auto& ref: lkms ) {
        sql.prepare( "INSERT INTO MSLock VALUES(?,?,?,?,?,?,?)" );
        int col = 1;
        sql.bind( col++ ) = objid;
        sql.bind( col++ ) = fcn;
        sql.bind( col++ ) = rowid;
        sql.bind( col++ ) = ref.formula();
        sql.bind( col++ ) = ref.exactMass();
        sql.bind( col++ ) = ref.matchedMass();
        sql.bind( col++ ) = ref.time();
        if ( sql.step() != adfs::sqlite_done )
            return false;
    }
    sql.commit();
    return true;
}

// static
bool
AcquiredConf::insert( adfs::sqlite& dbf
                      , uint64_t objid
                      , uint64_t pobjid
                      , const std::wstring& dataInterpreterClsid
                      , uint64_t trace_method
                      , uint64_t spectrometer_type
                      , const std::wstring& trace_id
                      , const std::wstring& trace_display_name
                      , const std::wstring& axis_x_label
                      , const std::wstring& axis_y_label
                      , uint64_t axis_x_decimals
                      , uint64_t axis_y_decimals ) 
{
    adfs::stmt sql( dbf );

    sql.prepare( 
        "INSERT INTO AcquiredConf VALUES(\
:objid                 \
,:pobjid               \
,:uuid                 \
,:dataInterpreterClsid \
,:trace_method         \
,:spectrometer         \
,:trace_id             \
,:trace_display_name   \
,:axis_x_label         \
,:axis_y_label         \
,:axis_x_decimails     \
,:axis_y_decimals      \
)" );
    
    static boost::uuids::uuid nullid { {(0)} } ;
    int col = 1;
    sql.bind( col++ ) = objid;
    sql.bind( col++ ) = pobjid;
    sql.bind( col++ ) = nullid;
    sql.bind( col++ ) = dataInterpreterClsid;
    sql.bind( col++ ) = trace_method;
    sql.bind( col++ ) = spectrometer_type;
    sql.bind( col++ ) = trace_id;
    sql.bind( col++ ) = trace_display_name;
    sql.bind( col++ ) = axis_x_label;
    sql.bind( col++ ) = axis_y_label;
    sql.bind( col++ ) = axis_x_decimals;
    sql.bind( col++ ) = axis_y_decimals;

    if ( sql.step() == adfs::sqlite_done ) {
        return true;
    } else
        sql.reset();
	return false;
}


// static
bool
AcquiredConf::insert( adfs::sqlite& dbf
                      , uint32_t objid
                      , uint32_t pobjid
                      , const boost::uuids::uuid& uuid
                      , const std::string& dataInterpreterClsid
                      , uint32_t trace_method
                      , uint32_t spectrometer_type
                      , const char * trace_id
                      , const char * trace_display_name
                      , const char * axis_x_label
                      , const char * axis_y_label
                      , uint32_t axis_x_decimals
                      , uint32_t axis_y_decimals ) 
{
    adfs::stmt sql( dbf );

    sql.prepare( 
        "INSERT INTO AcquiredConf VALUES(\
:objid                 \
,:pobjid               \
,:uuid                 \
,:dataInterpreterClsid \
,:trace_method         \
,:spectrometer         \
,:trace_id             \
,:trace_display_name   \
,:axis_x_label         \
,:axis_y_label         \
,:axis_x_decimails     \
,:axis_y_decimals      \
)" );
    
    sql.begin();

    int col = 1;
    sql.bind( col++ ) = objid;
    sql.bind( col++ ) = pobjid;
    sql.bind( col++ ) = uuid;
    sql.bind( col++ ) = dataInterpreterClsid;
    sql.bind( col++ ) = trace_method;
    sql.bind( col++ ) = spectrometer_type;
    sql.bind( col++ ) = std::string(trace_id);
    sql.bind( col++ ) = std::string(trace_display_name);
    sql.bind( col++ ) = std::string(axis_x_label);
    sql.bind( col++ ) = std::string(axis_y_label);
    sql.bind( col++ ) = axis_x_decimals;
    sql.bind( col++ ) = axis_y_decimals;

    if ( sql.step() == adfs::sqlite_done ) {
        sql.commit();
        return true;
    } else
        sql.reset();
	return false;
}

// static
bool
AcquiredConf::insert( adfs::sqlite& dbf, const data& d )
{
    return insert( dbf
                   , d.objid
                   , d.pobjid
                   , d.dataInterpreterClsid
                   , d.trace_method
                   , d.spectrometer
                   , d.trace_id
                   , d.trace_display_name
                   , d.axis_x_label
                   , d.axis_y_label
                   , d.axis_x_decimals
                   , d.axis_y_decimals );
}

AcquiredFormatVersion
AcquiredConf::formatVersion( adfs::sqlite& dbf )
{
    adfs::stmt sql( dbf );

    if ( sql.prepare( "PRAGMA TABLE_INFO(AcquiredConf)" ) ) {
        while ( sql.step() == adfs::sqlite_row ) {
            if ( sql.get_column_value< std::string >( 1 ) == "objuuid" ) 
                return format_v3;
        }
    }
    return format_v2;
}

bool
AcquiredConf::fetch( adfs::sqlite& db, std::vector< data >& vec )
{
    adfs::stmt sql( db );

    if ( formatVersion( db ) != format_v2 )
        return false;

    if ( sql.prepare(
        "SELECT objid, pobjid, dataInterpreterClsid, trace_method, spectrometer,\
         trace_id, trace_display_name, axis_x_label, axis_y_label, axis_x_decimals, axis_y_decimals FROM AcquiredConf" ) ) {

        while ( sql.step() == adfs::sqlite_row ) {

            data d;

            try {
                d.objid = sql.get_column_value<int64_t>( 0 );
                d.pobjid = sql.get_column_value<int64_t>( 1 );
                d.dataInterpreterClsid = sql.get_column_value<std::wstring>( 2 );
                d.trace_method = sql.get_column_value<int64_t>( 3 );
                d.spectrometer = sql.get_column_value<int64_t>( 4 );
                d.trace_id = sql.get_column_value<std::wstring>( 5 );
                d.trace_display_name = sql.get_column_value<std::wstring>( 6 );
                d.axis_x_label = sql.get_column_value<std::wstring>( 7 );
                d.axis_y_label = sql.get_column_value<std::wstring>( 8 );
                d.axis_x_decimals = sql.get_column_value<int64_t>( 9 );
                d.axis_y_decimals = sql.get_column_value<int64_t>( 10 );
            } catch ( std::bad_cast& ) {
                // ignore
            }
            vec.push_back( d );
        }
    }
    return true;
}

AcquiredConf::data::data() : objid( 0 )
                           , pobjid( 0 )
                           , trace_method( 0 )
                           , spectrometer( 0 )
                           , axis_x_decimals( 0 )
                           , axis_y_decimals( 0 )
{
}

AcquiredConf::data::data( const data& t ) : objid( t.objid )
                                          , pobjid( t.pobjid )
                                          , trace_method( t.trace_method )
                                          , spectrometer( t.spectrometer )
                                          , dataInterpreterClsid( t.dataInterpreterClsid )
                                          , trace_id( t.trace_id )
                                          , trace_display_name( t.trace_display_name )
                                          , axis_x_label( t.axis_x_label )
    , axis_y_label( t.axis_y_label )
    , axis_x_decimals( t.axis_x_decimals )
    , axis_y_decimals( t.axis_y_decimals )
{
}

