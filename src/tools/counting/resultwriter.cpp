/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "constants.hpp"
#include <acqrscontrols/threshold_result.hpp>
#include <acqrscontrols/ap240/waveform.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <ratio>

ResultWriter::ResultWriter( adfs::sqlite& db ) : db_( db )
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

ResultWriter::~ResultWriter()
{
}

ResultWriter&
ResultWriter::operator << ( std::shared_ptr< const acqrscontrols::threshold_result_< acqrscontrols::ap240::waveform > > rp )
{
    if ( rp ) {
        auto wp = rp->data();  // waveform
                
        do {
            adfs::stmt sql( db_ );
            
            sql.prepare( "INSERT INTO trigger ( id,protocol,timeSinceEpoch,elapsedTime,events,threshold,algo ) VALUES (?,?,?,?,?,?,?)" );
            int id(1);
            sql.bind( id++ ) = wp->serialnumber_;
            sql.bind( id++ ) = wp->method_.protocolIndex();
            sql.bind( id++ ) = wp->timeSinceEpoch_;
            sql.bind( id++ ) = wp->meta_.initialXTimeSeconds;
            sql.bind( id++ ) = wp->wellKnownEvents_;
            sql.bind( id++ ) = rp->threshold_level();  // V
            sql.bind( id++ ) = int( rp->algo() );
            if ( sql.step() != adfs::sqlite_done ) {
                ADDEBUG() << "sql error";
                return *this;
            }
        } while ( 0 );
        
        do {
            adfs::stmt sql( db_ );
            for ( auto& idx : rp->indecies2() ) {
                
                sql.prepare( "INSERT INTO peak"
                             " (idTrigger,peak_time,peak_intensity,front_offset,front_intensity,back_offset,back_intensity )"
                             " VALUES (?,?,?,?,?,?,?)" );
                int id = 1;
                auto apex  = wp->xy( idx.apex );                        
                sql.bind( id++ ) = wp->serialnumber_;                                   // idTrigger
                sql.bind( id++ ) = apex.first;                                          // peak_time
                sql.bind( id++ ) = wp->toVolts( apex.second ) * std::milli::den;        // peak_intensity, mV
                sql.bind( id++ ) = idx.first - idx.apex;                                // distance between front and apex
                sql.bind( id++ ) = wp->toVolts( (*wp)[ idx.first ] ) * std::milli::den; // front mV
                sql.bind( id++ ) = idx.second - idx.apex;                               // distance between apex and back
                sql.bind( id++ ) = wp->toVolts( (*wp)[ idx.second ] ) * std::milli::den;// front mV
                if ( sql.step() != adfs::sqlite_done ) {
                    ADDEBUG() << "sql error";
                    return *this;
                }     
            }
        } while ( 0 );
    }    
    return *this;
}

void
ResultWriter::commitData() 
{
}

