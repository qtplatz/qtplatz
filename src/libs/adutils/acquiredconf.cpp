/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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
#include <adfs/sqlite.hpp>
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
:objid                                           \
,:pobjid                                         \
,:dataInterpreterClsid                           \
,:trace_method                                   \
,:spectrometer                                   \
,:trace_id                                       \
,:trace_display_name                             \
,:axis_x_label                                   \
,:axis_y_label                                   \
,:axis_x_decimails                               \
,:axis_y_decimals                                \
)" );
    
    sql.begin();

    sql.bind( 1 ) = objid;
    sql.bind( 2 ) = pobjid;
    sql.bind( 3 ) = dataInterpreterClsid;
    sql.bind( 4 ) = trace_method;
    sql.bind( 5 ) = spectrometer_type;
    sql.bind( 6 ) = trace_id;
    sql.bind( 7 ) = trace_display_name;
    sql.bind( 8 ) = axis_x_label;
    sql.bind( 9 ) = axis_y_label;
    sql.bind( 10 ) = axis_x_decimals;
    sql.bind( 11 ) = axis_y_decimals;

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

bool
AcquiredConf::fetch( adfs::sqlite& db, std::vector< data >& vec )
{
    adfs::stmt sql( db );

    if ( sql.prepare(
             "SELECT objid, pobjid, dataInterpreterClsid, trace_method, spectrometer,\
 trace_id, trace_display_name, axis_x_label, axis_y_label, axis_x_decimals, axis_y_decimals FROM AcquiredConf" ) ) {

        while ( sql.step() == adfs::sqlite_row ) {
            
			data d;
            
			try {
                d.objid  = sql.get_column_value<int64_t>( 0 );
                d.pobjid = sql.get_column_value<int64_t>( 1 );
                d.dataInterpreterClsid = sql.get_column_value<std::wstring>( 2 );
                d.trace_method = sql.get_column_value<int64_t>( 3 );
                d.spectrometer = sql.get_column_value<int64_t>( 4 );
                d.trace_id = sql.get_column_value<std::wstring>( 5 );
                d.trace_display_name = sql.get_column_value<std::wstring>( 6 );
                d.axis_x_label  = sql.get_column_value<std::wstring>( 7 );
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

