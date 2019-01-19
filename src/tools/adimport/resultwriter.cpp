/**************************************************************************
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "resultwriter.hpp"
#include <adcontrols/counting/trigger_data.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/counting/counting_result.hpp>
#include <adportable/debug.hpp>
#include <ratio>

using namespace tools;

resultwriter::resultwriter( adfs::sqlite& db ) : db_( db )
{
    adfs::stmt sql( db_ );

    sql.exec( "PRAGMA synchronous = OFF" );
    sql.exec( "PRAGMA journal_mode = MEMORY" );
    sql.exec( "PRAGMA page_size = 8192" );

    sql.exec( "DROP TABLE IF EXISTS peak" );
    sql.exec( "DROP TABLE IF EXISTS trigger" );

    sql.exec(
        "CREATE TABLE \
trigger (                 \
id INTEGER PRIMARY KEY    \
, protocol INTEGER        \
, timeSinceEpoch INTEGER  \
, elapsedTime REAL        \
, events INTEGER          \
, threshold REAL          \
, algo INTEGER )" );

    sql.exec(
        "CREATE TABLE \
peak (                    \
idTrigger INTEGER         \
,peak_time REAL           \
,peak_intensity REAL      \
,front_offset INTEGER     \
,front_intensity REAL     \
,back_offset INTEGER      \
,back_intensity REAL      \
,FOREIGN KEY( idTrigger ) REFERENCES trigger( id ))" );
}

resultwriter::~resultwriter()
{
}

bool
resultwriter::insert( std::shared_ptr< const adcontrols::MassSpectrum > ms
                      , const adcontrols::counting::trigger_data& t
                      , adportable::counting::counting_result&& rp )
{
    do {
        adfs::stmt sql( db_ );

        sql.prepare( "INSERT INTO trigger ( id,protocol,timeSinceEpoch,elapsedTime,events,threshold,algo ) VALUES (?,?,?,?,?,?,?)" );
        int id(1);
        sql.bind( id++ ) = t.serialnumber;
        sql.bind( id++ ) = t.protocolIndex;
        sql.bind( id++ ) = t.timeSinceEpoch;
        sql.bind( id++ ) = ms->getTime( 0 );
        sql.bind( id++ ) = t.wellKnownEvents;
        sql.bind( id++ ) = t.thresholdLevel;
        sql.bind( id++ ) = t.algo;
        if ( sql.step() != adfs::sqlite_done ) {
            ADDEBUG() << "sql error";
            return false;
        }
    } while ( 0 );

    do {
        adfs::stmt sql( db_ );
        for ( auto& idx : rp.indices2() ) {

            sql.prepare( "INSERT INTO peak"
                         " (idTrigger,peak_time,peak_intensity,front_offset,front_intensity,back_offset,back_intensity )"
                         " VALUES (?,?,?,?,?,?,?)" );
            int id = 1;
            sql.bind( id++ ) = t.serialnumber;                                      // idTrigger
            sql.bind( id++ ) = ms->getTime( idx.apex );                             // peak_time
            sql.bind( id++ ) = ms->getIntensity( idx.apex );                        // peak_intensity, mV
            sql.bind( id++ ) = idx.first - idx.apex;                                // distance between front and apex
            sql.bind( id++ ) = ms->getIntensity( idx.first );                       // front mV
            sql.bind( id++ ) = idx.second - idx.apex;                               // distance between apex and back
            sql.bind( id++ ) = ms->getIntensity( idx.second );                      // front mV
            if ( sql.step() != adfs::sqlite_done ) {
                ADDEBUG() << "sql error";
                return false;
            }
        }
    } while ( 0 );

    return true;
}

void
resultwriter::commit()
{
}
