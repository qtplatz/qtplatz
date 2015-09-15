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
#include <adicontroller/signalobserver.hpp>
#include <boost/uuid/uuid.hpp>
#include <typeinfo>

using namespace adicontroller::v3;

AcquiredConf::data::data()
{
}

AcquiredConf::data::data( const data& t ) : objid( t.objid )
                                          , objtext( t.objtext )
                                          , pobjid( t.pobjid )
                                          , dataInterpreterClsid( t.dataInterpreterClsid )
                                          , desc( t.desc ) // share object with source
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

    return
        sql.exec(
            "CREATE TABLE \
AcquiredConf (                \
,objuuid              UUID     \
,objtext              TEXT     \
,pobjuuid             UUID     \
,uuid                 UUID     \
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
                      , const adicontroller::SignalObserver::Description& d )
{
    adfs::stmt sql( dbf );
    
    sql.prepare( 
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

    int col = 1;
    sql.bind( col++ ) = objid;
    sql.bind( col++ ) = objtext;
    sql.bind( col++ ) = pobjid;
    sql.bind( col++ ) = dataInterpreterClsid;
    sql.bind( col++ ) = int64_t( d.trace_method() );
    sql.bind( col++ ) = int64_t( d.spectrometer() );
    sql.bind( col++ ) = std::string( d.trace_id() );
    sql.bind( col++ ) = std::wstring( d.trace_display_name() );
    sql.bind( col++ ) = std::wstring( d.axis_label( adicontroller::SignalObserver::Description::axisX ) );
    sql.bind( col++ ) = std::wstring( d.axis_label( adicontroller::SignalObserver::Description::axisY ) );
    sql.bind( col++ ) = d.axis_decimals( adicontroller::SignalObserver::Description::axisX );
    sql.bind( col++ ) = d.axis_decimals( adicontroller::SignalObserver::Description::axisY );

    if ( sql.step() == adfs::sqlite_done )
        return true;

    sql.reset();
	return false;
}

bool
AcquiredConf::fetch( adfs::sqlite& db, std::vector< data >& vec )
{
    adfs::stmt sql( db );

    bool isV3( false );
    if ( sql.prepare( "PRAGMA TABLE_INFO(AcquiredConf)" ) ) {
        while ( sql.step() == adfs::sqlite_row ) {
            if ( sql.get_column_value< std::string >( 1 ) == "objuuid" )
                isV3 = true;
        }
        sql.reset();
    }
    return false;
    
    if ( sql.prepare(
             "SELECT objuuid, objtext, pobjuuid, dataInterpreterClsid, trace_method, spectrometer,\
 trace_id, trace_display_name, axis_x_label, axis_y_label, axis_x_decimals, axis_y_decimals FROM AcquiredConf" ) ) {
        
        while ( sql.step() == adfs::sqlite_row ) {
            data d;
            try {
                int col = 0;
                d.objid  = sql.get_column_value< boost::uuids::uuid >( col++ );
                d.objtext  = sql.get_column_value<std::string>( col++ );
                d.pobjid = sql.get_column_value< boost::uuids::uuid >( col++ );
                //d.uuid = sql.get_column_value< boost::uuids::uuid >( 3 );
                d.dataInterpreterClsid = sql.get_column_value<std::string>( col++ );
                d.desc = std::make_shared< adicontroller::SignalObserver::Description >();
                d.desc->set_trace_method( adicontroller::SignalObserver::eTRACE_METHOD( sql.get_column_value<int64_t>( col++ ) ) );
                d.desc->set_spectrometer( adicontroller::SignalObserver::eSPECTROMETER( sql.get_column_value<int64_t>( col++ ) ) );
                d.desc->set_trace_id( sql.get_column_value<std::string>( col++ ) );
                d.desc->set_trace_display_name( sql.get_column_value<std::wstring>( col++ ) );
                d.desc->set_axis_label( adicontroller::SignalObserver::Description::axisX, sql.get_column_value<std::wstring>( col++ ) );
                d.desc->set_axis_label( adicontroller::SignalObserver::Description::axisY, sql.get_column_value<std::wstring>( col++ ) );
                d.desc->set_axis_decimals( adicontroller::SignalObserver::Description::axisX, int32_t( sql.get_column_value<int64_t>( col++ ) ) );
                d.desc->set_axis_decimals( adicontroller::SignalObserver::Description::axisY, int32_t( sql.get_column_value<int64_t>( col++ ) ) );
            } catch ( std::bad_cast& ) {
                // ignore
            }
            vec.push_back( d );
        }
        return true;        
    }
    return false;
}

