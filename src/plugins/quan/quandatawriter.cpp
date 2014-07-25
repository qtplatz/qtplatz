/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quandatawriter.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quansequence.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <adfs/adfs.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <adfs/cpio.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportable/profile.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

using namespace quan;

QuanDataWriter::~QuanDataWriter()
{
}

QuanDataWriter::QuanDataWriter( const std::wstring& path ) : path_( path ) 
{
}

bool
QuanDataWriter::open()
{
    if ( !boost::filesystem::exists( path_ ) ) {
        if ( !fs_.create( path_.c_str() ) )
            return false;
    } else {
        if ( !fs_.mount( path_.c_str() ) )
            return false;
    }

    return true;
}

adfs::file
QuanDataWriter::write( const adcontrols::MassSpectrum& ms, const std::wstring& tittle )
{
    if ( adfs::folder folder = fs_.addFolder( L"/Processed/Spectra" ) ) {
        if ( adfs::file file = folder.addFile( adfs::create_uuid(), tittle ) ) {
            file.dataClass( ms.dataClass() );
            if ( adfs::cpio< adcontrols::MassSpectrum >::save( ms, file ) )
                file.commit();
            return file;
        }
    }
    return adfs::file();
}

adfs::file
QuanDataWriter::write( const adcontrols::ProcessMethod& pm )
{
    if ( adfs::folder folder = fs_.addFolder( L"/Processed/Quan" ) ) {
        if ( adfs::file file = folder.addFile( adfs::create_uuid(), L"QuanMethod" ) ) {
            file.dataClass( pm.dataClass() );
            if ( adfs::cpio< adcontrols::ProcessMethod >::save( pm, file ) )
                file.commit();
            return file;
        }
    }
    return adfs::file();
}

adfs::file
QuanDataWriter::write( const adcontrols::QuanSequence& t)
{
    if ( adfs::folder folder = fs_.addFolder( L"/Processed/Quan" ) ) {
        if ( adfs::file file = folder.addFile( adfs::create_uuid(), L"QuanSequence" ) ) {
            file.dataClass( t.dataClass() );
            if ( adfs::cpio< adcontrols::QuanSequence >::save( t, file ) )
                file.commit();
            return file;
        }
    }
    return adfs::file();
}

adfs::file
QuanDataWriter::write( const adcontrols::QuanSample& t )
{
    if ( adfs::folder folder = fs_.addFolder( L"/Processed/Quan" ) ) {
        if ( adfs::file file = folder.addFile( adfs::create_uuid(), L"QuanSample" ) ) {
            file.dataClass( t.dataClass() );
            if ( adfs::cpio< adcontrols::QuanSample >::save( t, file ) )
                file.commit();
            return file;
        }
    }
    return adfs::file();
}

bool
QuanDataWriter::create_table()
{
    adfs::sqlite & db = fs_.db();

    adfs::stmt sql( db );

    // sql.exec( "PRAGMA FOREIGN_KEYS = ON" );

    bool result( false );

    result |= sql.exec(
        "CREATE TABLE idAudit ( \
uuid                    TEXT    \
,digest                 TEXT    \
,dateCreated            DATE    \
,idComputer             TEXT    \
,idCreatedBy            TEXT    \
,nameCreatedBy          TEXT)" );


    result |= sql.exec( 
        "CREATE TABLE QuanMethod ( \
idAudit                 INTEGER \
,equation               INTEGER \
,polynomialOrder        INTEGER \
,isChromatogram         INTEGER \
,isWighting             INTEGER \
,isBracketing           INTEGER \
,bracketing             INTEGER \
,weighting              INTEGER \
,isISTD                 INTEGER \
,levels                 INTEGER \
,replicates             INTEGER \
,quanMethodFilename     TEXT    \
,quanCompoundsFilename  TEXT    \
,quanSequenceFilename   TEXT    \
,FOREIGN KEY ( idAudit ) REFERENCES idAudit ( rowid ) )" );

    result |= sql.exec( "CREATE TABLE QuanSequence (uuid  TEXT, outfile TEXT)" );

    result |= sql.exec(
        "CREATE TABLE QuanSample (\
sequenceId       INTEGER               \
,name            TEXT                  \
,dataType        TEXT                  \
,dataSource      TEXT                  \
,dataGuid        TEXT                  \
,sampleType      INTEGER               \
,level           INTEGAR               \
,ISTDID          INTEGAR               \
,injVol          REAL                  \
,amountsAdded    REAL                  \
,channel         INTAGER               \
,dataGeneration  INTEGER               \
,scan_range_first INTEGER              \
,scan_range_second INTEGER                                      \
,FOREIGN KEY( SEQUENCEID ) REFERENCES QuanSample( ROWID ))" );

    result |= sql.exec(
        "CREATE TABLE QuanISTD ( \
sampleId        INTEGER               \
,ISTDID         INTEGAR                                  \
,amounts        REAL                                     \
,FOREIGN KEY( sampleId ) REFERENCES QuanSequence( ROWID ) \
)" );

    result |= sql.exec(
        "CREATE TABLE QuanCompound ( \
uuid            TEXT                 \
,uniqId         INTEGER              \
,display_name   TEXT                 \
,formula        TEXT                 \
,isISTD         INTEGER              \
,idISTD         INTEGER              \
,levels         INTEGER              \
,mass           REAL                 \
,tR             REAL                 \
,description    TEXT                 \
,criteria       REAL                 \
,lkms_reference INTEGER              \
,UNIQUE(uuid,uniqId)                 \
)" );

    result |= sql.exec(
        "CREATE TABLE QuanAmount ( CompoundId INTEGER, level INTEGER, amount REAL \
, FOREIGN KEY( CompoundId ) REFERENCES QuanCompound ( rowid ) )" );

    result |= sql.exec(
        "CREATE TABLE QuanResponse ( \
sampleId        INTEGAR                   \
,idx            INTEGER                   \
,fcn            INTEGAR                   \
,compoundId     INTEGAR                   \
,intensity      REAL                      \
,mass           REAL                      \
,tR             REAL                                        \
,FOREIGN KEY( sampleId ) REFERENCES QuanSample ( rowid )    \
,FOREIGN KEY( compoundId ) REFERENCES QuanCompound ( rowid ) \
)" );

    return result;
}

bool
QuanDataWriter::drop_table()
{
    adfs::stmt sql( fs_.db() );

    sql.exec( "DROP TABLE QuanResponse" );
    sql.exec( "DROP TABLE QuanCompound" );
    sql.exec( "DROP TABLE QuanAmount" );
    sql.exec( "DROP TABLE QuanISTD" );
    sql.exec( "DROP TABLE QuanSample" );
    sql.exec( "DROP TABLE QuanSequence" );
    sql.exec( "DROP TABLE QuanMethod" );
    sql.exec( "DROP TABLE idAudit" );

    return true;
}

bool
QuanDataWriter::insert_table( const adcontrols::QuanMethod& t )
{
    adfs::stmt sql( fs_.db() );

    if ( sql.prepare("INSERT INTO idAudit VALUES (?,?,?,?,?,?)" ) ) {
        sql.bind( 1 ) = boost::lexical_cast< std::string >( t.ident().uuid() );
        sql.bind( 2 ) = std::string( t.ident().digest() );
        sql.bind( 3 ) = std::string( t.ident().dateCreated() );
        sql.bind( 4 ) = std::wstring( t.ident().idComputer() );
        sql.bind( 5 ) = std::wstring( t.ident().idCreatedBy() );
        sql.bind( 6 ) = std::wstring( t.ident().nameCreatedBy() );
        if ( sql.step() != adfs::sqlite_done )
            ADTRACE() << "sql error";
    }

    if ( sql.prepare( "INSERT INTO QuanMethod SELECT rowid,?,?,?,?,?,?,?,?,?,?,?,?,? from idAudit WHERE uuid = :uuid" ) ) {
        sql.bind( 1 ) = int64_t(t.equation());
        sql.bind( 2 ) = int64_t(t.polynomialOrder());
        sql.bind( 3 ) = int64_t(t.isChromatogram());
        sql.bind( 4 ) = int64_t(t.isWeighting());
        sql.bind( 5 ) = int64_t(t.isBracketing());
        sql.bind( 6 ) = int64_t(t.bracketing());
        sql.bind( 7 ) = int64_t(t.weighting());
        sql.bind( 8 ) = int64_t(t.isInternalStandard());
        sql.bind( 9 ) = t.levels();
        sql.bind( 10 ) = t.replicates();
        sql.bind( 11 ) = std::wstring( t.quanMethodFilename() );
        sql.bind( 12 ) = std::wstring( t.quanCompoundsFilename() );
        sql.bind( 13 ) = std::wstring( t.quanSequenceFilename() );
        sql.bind( 14 ) = boost::lexical_cast< std::string >( t.ident().uuid() ); // :uuid (idAudit.rowid)
        if ( sql.step() != adfs::sqlite_done )
            ADTRACE() << "sql error";
    }
    return true;
}

bool
QuanDataWriter::insert_table( const adcontrols::QuanSequence& t )
{
    adfs::stmt sql( fs_.db() );

    if ( sql.prepare( "INSERT INTO QuanSequence VALUES(?,?)" ) ) {
        sql.bind( 1 ) = boost::lexical_cast< std::string >( t.uuid() );
        sql.bind( 2 ) = std::wstring( t.outfile() );
        if ( sql.step() != adfs::sqlite_done )
            ADTRACE() << "sql error";
    }

    for ( auto& sample: t ) {
        if ( sql.prepare( "INSERT INTO QuanSample SELECT rowid, ?,?,?,?,?,?,?,?,?,?,?,?,? FROM QuanSequence WHERE uuid = :uuid" ) ) {
            sql.bind( 1 ) = std::wstring( sample.name() );
            sql.bind( 2 ) = std::wstring( sample.dataType() );
            sql.bind( 3 ) = std::wstring( sample.dataSource() );
            sql.bind( 4 ) = std::wstring( sample.dataGuid() );
            sql.bind( 5 ) = int64_t( sample.sampleType() );
            sql.bind( 6 ) = sample.level();
            sql.bind( 7 ) = sample.istdId();
            sql.bind( 8 ) = sample.injVol();
            sql.bind( 9  ) = sample.addedAmounts();
            sql.bind( 10 ) = sample.channel();
            sql.bind( 11 ) = int64_t( sample.dataGeneration() );
            sql.bind( 12 ) = sample.scan_range_first();
            sql.bind( 13 ) = sample.scan_range_second();
            sql.bind( 14 ) = boost::lexical_cast< std::string >( t.uuid() ); // :uuid
            //const std::vector< quan::ISTD >& istd() const;
            //void istd( const std::vector< quan::ISTD >& );
            if ( sql.step() != adfs::sqlite_done )
                ADTRACE() << "sql error";
        }
    }

    return true;
}

bool
QuanDataWriter::insert_table( const adcontrols::QuanCompounds& t )
{
    adfs::stmt sql( fs_.db() );

    std::string uuid = boost::lexical_cast<std::string>(t.uuid());

    for ( auto& c: t ) {

        if ( sql.prepare( "INSERT INTO QuanCompound VALUES(?,?,?,?,?,?,?,?,?,?,?,?)" ) ) {
            int row = 1;
            sql.bind( row++ ) = uuid;
            sql.bind( row++ ) = c.uniqId();
            sql.bind( row++ ) = std::wstring( c.display_name() );
            sql.bind( row++ ) = std::wstring( c.formula() );        
            sql.bind( row++ ) = int64_t( c.isISTD() );
            sql.bind( row++ ) = int64_t( c.idISTD() );
            sql.bind( row++ ) = int64_t( c.levels() );
            sql.bind( row++ ) = c.mass();
            sql.bind( row++ ) = c.tR();
            sql.bind( row++ ) = std::wstring( c.description() );
            sql.bind( row++ ) = c.criteria();
            sql.bind( row++ ) = int64_t(0); // lock mass flag
            
            if ( sql.step() != adfs::sqlite_row )
                ADTRACE() << "sql error";
        }

        for ( int i = 0; i < c.levels(); ++i ) {
            if ( sql.prepare(
                     "\
INSERT INTO QuanAmount (CompoundId, level, amount) \
SELECT rowid,:level,:amount FROM QuanCompound      \
WHERE uuid = :uuid and uniqId = :uniqId" ) ) {
                sql.bind(1) = i;                 // :level
                sql.bind(2) = c.amounts()[i];    // :amount
                sql.bind(3) = uuid;              // QuanCompound.uuid
                sql.bind(4) = c.uniqId();        // QuanCompound.uniqId
                if ( sql.step() != adfs::sqlite_done )
                    ADTRACE() << "sql error";                    
            }
        }
    }
    return false;
}

bool
QuanDataWriter::insert_table( const adcontrols::QuanSample& t )
{
    return false;
}
