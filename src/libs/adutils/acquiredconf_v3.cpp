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

#include "acquiredconf_v3.hpp"
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <boost/uuid/uuid.hpp>
#include <cassert>
#include <typeinfo>

using namespace adutils::v3;

AcquiredConf::data::data() : rowid(0)
                           , objid( {0} )
                           , objtext( "" )
                           , pobjid( boost::uuids::uuid{0} )
{
}

AcquiredConf::data::data( const data& t ) : rowid( t.rowid )
                                          , objid( t.objid )
                                          , objtext( t.objtext )
                                          , pobjid( t.pobjid )
                                          , dataInterpreterClsid( t.dataInterpreterClsid )
                                          , trace_method( t.trace_method )
                                          , spectrometer( t.spectrometer )
                                          , trace_id( t.trace_id )
                                          , trace_display_name( t.trace_display_name )
                                          , axis_label_x( t.axis_label_x )
                                          , axis_label_y( t.axis_label_y )
                                          , axis_decimals_x( t.axis_decimals_x )
                                          , axis_decimals_y( t.axis_decimals_y )
{
}

AcquiredConf::AcquiredConf()
{
}

// static
bool
AcquiredConf::create_table_v3( adfs::sqlite& db )
{
    adfs::stmt sql( db );

    sql.exec(
        "CREATE TABLE \
ScanLaw (             \
 objuuid UUID PRIMARY KEY \
,objtext TEXT         \
,acclVoltage REAL     \
,tDelay  REAL )"
        );

    return
        sql.exec(
            "CREATE TABLE \
AcquiredConf (                \
 objuuid              UUID     \
,objtext              TEXT     \
,pobjuuid             UUID     \
,dataInterpreterClsid TEXT     \
,trace_method         INTEGER  \
,spectrometer         INTEGER  \
,trace_id             TEXT     \
,trace_display_name   TEXT     \
,axis_x_label         TEXT     \
,axis_y_label         TEXT     \
,axis_x_decimals      INTEGER  \
,axis_y_decimals      INTEGER  \
)"
            );
}

// static
bool
AcquiredConf::insert( adfs::sqlite& dbf
                      , const boost::uuids::uuid& objid
                      , const std::string& objtext
                      , const boost::uuids::uuid& pobjid
                      , const std::string& dataInterpreterClsid
                      , int trace_method
                      , int spectrometer
                      , const std::string& trace_id
                      , const std::wstring& trace_display_name
                      , const std::wstring& axis_label_x
                      , const std::wstring& axis_label_y
                      , int axis_decimals_x
                      , int axis_decimals_y )
{
    adfs::stmt sql( dbf );
    
    bool success = sql.prepare(
        "INSERT INTO \
 AcquiredConf VALUES(\
:objuuid             \
,:objtext            \
,:pobjuuid           \
,:dataInterpreterClsid\
,:trace_method        \
,:spectrometer        \
,:trace_id            \
,:trace_display_name  \
,:axis_x_label        \
,:axis_y_label        \
,:axis_x_decimails    \
,:axis_y_decimals     \
)" );

    if ( success ) {

        int col = 1;
        sql.bind( col++ ) = objid;
        sql.bind( col++ ) = objtext;
        sql.bind( col++ ) = pobjid;
        sql.bind( col++ ) = dataInterpreterClsid;
        sql.bind( col++ ) = int64_t( trace_method );
        sql.bind( col++ ) = int64_t( spectrometer );
        sql.bind( col++ ) = trace_id;
        sql.bind( col++ ) = trace_display_name;
        sql.bind( col++ ) = axis_label_x;    // std::wstring( d.axis_label( adicontroller::SignalObserver::Description::axisX ) );
        sql.bind( col++ ) = axis_label_y;    // std::wstring( d.axis_label( adicontroller::SignalObserver::Description::axisY ) );
        sql.bind( col++ ) = axis_decimals_x; // d.axis_decimals( adicontroller::SignalObserver::Description::axisX );
        sql.bind( col++ ) = axis_decimals_y; // d.axis_decimals( adicontroller::SignalObserver::Description::axisY );

        if ( sql.step() == adfs::sqlite_done )
            return true;
    }

    sql.reset();
	return false;
}

// static
bool
AcquiredConf::insert( adfs::sqlite& dbf, const boost::uuids::uuid& objid, const data& d )
{
    adfs::stmt sql( dbf );
    
    bool success = sql.prepare(
        "INSERT INTO \
 AcquiredConf VALUES(\
:objuuid             \
,:objtext            \
,:pobjuuid           \
,:dataInterpreterClsid\
,:trace_method        \
,:spectrometer        \
,:trace_id            \
,:trace_display_name  \
,:axis_x_label        \
,:axis_y_label        \
,:axis_x_decimails    \
,:axis_y_decimals     \
)" );

    assert( objid == d.objid );

    if ( success ) {

        int col = 1;
        sql.bind( col++ ) = objid;
        sql.bind( col++ ) = d.objtext;
        sql.bind( col++ ) = d.pobjid;
        sql.bind( col++ ) = d.dataInterpreterClsid;
        sql.bind( col++ ) = int64_t( d.trace_method );
        sql.bind( col++ ) = int64_t( d.spectrometer );
        sql.bind( col++ ) = d.trace_id;
        sql.bind( col++ ) = d.trace_display_name;
        sql.bind( col++ ) = d.axis_label_x;
        sql.bind( col++ ) = d.axis_label_y;
        sql.bind( col++ ) = d.axis_decimals_x;
        sql.bind( col++ ) = d.axis_decimals_y;

        if ( sql.step() == adfs::sqlite_done )
            return true;
    }

    sql.reset();
	return false;
}

bool
AcquiredConf::fetch( adfs::sqlite& db, std::vector< data >& vec )
{
    adfs::stmt sql( db );

    if ( sql.prepare(
             "SELECT rowid, objuuid, objtext, pobjuuid, dataInterpreterClsid, trace_method, spectrometer,\
 trace_id, trace_display_name, axis_x_label, axis_y_label, axis_x_decimals, axis_y_decimals FROM AcquiredConf" ) ) {
        
        while ( sql.step() == adfs::sqlite_row ) {
            data d;
            try {
                int col = 0;
                d.rowid = sql.get_column_value< int64_t >( col++ );
                d.objid  = sql.get_column_value< boost::uuids::uuid >( col++ );
                d.objtext  = sql.get_column_value<std::string>( col++ );
                d.pobjid = sql.get_column_value< boost::uuids::uuid >( col++ );
                d.dataInterpreterClsid = sql.get_column_value<std::string>( col++ );
                d.trace_method = int( sql.get_column_value<int64_t>( col++ ) );
                d.spectrometer = int( sql.get_column_value<int64_t>( col++ ) );
                d.trace_id = sql.get_column_value<std::string>( col++ );
                d.trace_display_name = sql.get_column_value<std::wstring>( col++ );
                d.axis_label_x = sql.get_column_value<std::wstring>( col++ );
                d.axis_label_y = sql.get_column_value<std::wstring>( col++ );
                d.axis_decimals_x = int32_t( sql.get_column_value<int64_t>( col++ ) );
                d.axis_decimals_y = int32_t( sql.get_column_value<int64_t>( col++ ) );

            } catch ( std::bad_cast& ex ) {
                ADDEBUG() << ex.what();
                return false;
            }
            vec.push_back( d );
        }
        return true;        
    }
    return false;
}

