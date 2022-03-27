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

#include "mscalibio.hpp"
#include <adfs/sqlite.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/xml_serializer.hpp>
#include <adportable/debug.hpp>

using namespace adutils;

mscalibio::mscalibio()
{
}


// static
bool
mscalibio::create_table( adfs::sqlite& db )
{
    adfs::stmt sql( db );
    if ( sql.exec(
             "CREATE TABLE IF NOT EXISTS MSCalibration ("
             " calibrationUuid       UUID"
             ",massSpectrometerClsid UUID"
             ",mode                 INTEGER"
             ",dataClass            TEXT"
             ",date                 TEXT"
             ",data                 BLOB"
             ",xml                  TEXT"
             ",formula              TEXT"
             ")"
             ) )
		return true;

    ADDEBUG() << sql.errmsg();

	return false;
}


//static
bool
mscalibio::write( adfs::sqlite& db, const adcontrols::MSCalibrateResult& calibResult )
{
    adfs::stmt sql( db );

    sql.prepare( "SELECT name FROM sqlite_master WHERE type='table' AND name='MSCalibration'");
    if ( sql.step() != adfs::sqlite_row )
        create_table( db );

    auto calib = calibResult.calibration();

    const char * const query = "INSERT INTO MSCalibration "
        "(calibrationUuid,massSpectrometerClsid,mode,dataClass,date,data,xml,formula)"
        " VALUES (?,?,?,?,?,?,?,?)";

    std::string device;
    std::wostringstream xml;

    if ( adportable::binary::serialize<>()( calibResult, device ) && adportable::xml::serialize<>()( calibResult, xml ) ) {

        sql.prepare( query );

        sql.bind( 1 ) = calib.calibrationUuid();
        sql.bind( 2 ) = calib.massSpectrometerClsid();
        sql.bind( 3 ) = calib.mode();
        sql.bind( 4 ) = std::wstring( calibResult.dataClass() );
        sql.bind( 5 ) = calib.date();
        sql.bind( 6 ) = adfs::blob( device.size(), device.data() );
        sql.bind( 7 ) = xml.str();
        sql.bind( 8 ) = calib.formulaText( false );

        if ( sql.step() == adfs::sqlite_done )
            return true;

        ADDEBUG() << sql.errmsg();
    }

    return false;
}

//static
bool
mscalibio::read( adfs::sqlite& db, adcontrols::MSCalibrateResult& calibResult, const boost::uuids::uuid& massSpectrometerClsid )
{
    adfs::stmt sql( db );

    sql.prepare( "SELECT data FROM MSCalibration WHERE massSpectrometerClsid=? ORDER BY rowid DESC LIMIT 1" );
    sql.bind( 1 ) = massSpectrometerClsid;
    if ( sql.step() == adfs::sqlite_row ) {
        adfs::blob blob = sql.get_column_value< adfs::blob >( 0 );
        return adportable::binary::deserialize<>()( calibResult, reinterpret_cast< const char *>( blob.data() ), blob.size() );
    }

    return false;
}
