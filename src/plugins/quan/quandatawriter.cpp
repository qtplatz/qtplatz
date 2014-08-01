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

    bool result( true );

    result &= sql.exec(
        "CREATE TABLE idAudit (\
 id INTEGER PRIMARY KEY \
,uuid          TEXT \
,digest        TEXT \
,dateCreated   DATE \
,idComputer    TEXT \
,idCreatedBy   TEXT \
,nameCreatedBy TEXT \
,reason        TEXT\
,UNIQUE(uuid) )" );

    result &= sql.exec( 
        "CREATE TABLE QuanMethod (\
 id      INTEGER PRIMARY KEY \
,idAudit         INTEGER \
,uuid            TEXT \
,equation        INTEGER \
,polynomialOrder INTEGER \
,isChromatogram  INTEGER \
,isWeighting     INTEGER \
,isBracketing    INTEGER \
,bracketing      INTEGER \
,weighting       INTEGER \
,isISTD          INTEGER \
,levels          INTEGER \
,replicates      INTEGER \
,fnMethod        TEXT \
,fnCompounds     TEXT \
,fnSequence      TEXT \
,FOREIGN KEY ( idAudit ) REFERENCES idAudit ( id ) \
,FOREIGN KEY ( uuid ) REFERENCES idAudit ( uuid ) )" );

    result &= sql.exec( 
        "CREATE TABLE QuanSequence (\
 id      INTEGER PRIMARY KEY \
,idAudit INTEGER \
,uuid    TEXT \
,outfile TEXT \
,UNIQUE(uuid) \
,FOREIGN KEY ( idAudit ) REFERENCES idAudit ( id ) )" );

// QuanSample is corresponding to a sample, which is consisted from several compounds.
// It is equivalent to a chromatogram, or a spectrum.  Each peak detected from them are stroing into QuanResponse

    result &= sql.exec(
        "CREATE TABLE IF NOT EXISTS QuanSample (\
 id INTEGER PRIMARY KEY \
,idSequence      INTEGER \
,uuid            TEXT    \
,row             INTEGER \
,name            TEXT    \
,dataType        TEXT    \
,dataSource      TEXT    \
,dataGuid        TEXT    \
,sampleType      INTEGER \
,level           INTEGAR \
,ISTDID          INTEGAR \
,injVol          REAL    \
,amountsAdded    REAL    \
,channel         INTAGER \
,dataGeneration  INTEGER \
,data_first      INTEGER  \
,data_second     INTEGER \
,UNIQUE(uuid,row) \
,FOREIGN KEY( uuid ) REFERENCES QuanSequence( uuid ))" );


    result &= sql.exec(
        "CREATE TABLE QuanISTD (\
 idSample INTEGER \
,ISTDID  INTEGAR \
,amounts REAL    \
,FOREIGN KEY( idSample ) REFERENCES QuanSample( id ) \
)" );


// QuanCompound := table of compounds so that all compound in a table share uuid
// uniqId is unique within a table, but not unique across the tables
// 'id' is actual unique key within a database

    result &= sql.exec(
        "CREATE TABLE QuanCompound (\
 id  INTEGER PRIMARY KEY \
,uuid           TEXT     \
,uniqId         INTEGER  \
,display_name   TEXT     \
,formula        TEXT     \
,idISTD         INTEGER  \
,levels         INTEGER  \
,mass           REAL     \
,tR             REAL     \
,isLKMSRef      INTEGER  \
,isTimeRef      INTEGER  \
,isISTD         INTEGER  \
,description    TEXT     \
,criteria_0     REAL     \
,criteria_1     REAL     \
,UNIQUE(uuid,uniqId)     \
,FOREIGN KEY( uuid ) REFERENCES idAudit( uuid ))" );

    result &= sql.exec(
        "CREATE TABLE QuanAmount (\
 idCompound INTEGER \
,level INTEGER \
,amount REAL \
,FOREIGN KEY( idCompound ) REFERENCES QuanCompound ( id ) )" );

    result &= sql.exec(
        "CREATE TABLE QuanResponse ( \
 id     INTEGER PRIMARY KEY \
,idSample       INTEGAR \
,idx            INTEGER \
,fcn            INTEGAR \
,uniqId         INTEGAR \
,uniqGuid       TEXT    \
,intensity      REAL    \
,formula        TEXT    \
,mass           REAL    \
,tR             REAL    \
,calibId        INTEGER \
,amount         REAL    \
,FOREIGN KEY( idSample ) REFERENCES QuanSample ( id ) )" );

    result &= sql.exec("\
CREATE TABLE QuanCalib (\
 id INTEGER PRIMARY KEY\
,idAudit       INTEGER \
,uuid          TEXT \
,idCompound    INTEGER \
,uniqGuid      TEXT \
,uniqId        INTEGER \
,uuidMethod    TEXT \
,n             INTEGER \
,min_x         REAL \
,max_x         REAL \
,chisqr        REAL \
,a REAL, b REAL, c REAL, d REAL, e REAL, f REAL \
,date       TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL \
,FOREIGN KEY ( idCompound ) REFERENCES QuanCompound ( id ) \
,FOREIGN KEY ( uniqGuid, uniqId ) REFERENCES QuanCompound ( uuid,uniqId ) \
,FOREIGN KEY ( idAudit ) REFERENCES idAudit ( id ) \
,FOREIGN KEY ( uuid ) REFERENCES idAudit ( uuid ) \
)" );
    return result;
// ,FOREIGN KEY ( idCompound ) REFERENCES QuanCompound( id )\
// ,FOREIGN KEY ( idAudit ) REFERENCES idAudit( id )\

}

bool
QuanDataWriter::drop_table()
{
    adfs::stmt sql( fs_.db() );

    static const char * drop_order[] = {
        "QuanResponse"
        , "QuanSample"
        , "QuanSequence"
        , "QuanISTD"
        , "QuanAmount"
        , "QuanCompound"
        , "QuanMethod"
        , "idAudit" };

    std::vector< std::string > table_names;

    if ( sql.prepare( "SELECT name FROM sqlite_master WHERE type='table' AND ( name like 'Quan%' OR name = 'idAudit' )" ) ) {
        while ( sql.step() == adfs::sqlite_row )
            table_names.push_back( sql.get_column_value< std::string >( 0 ) );
    }
    for ( auto name: drop_order ) {
        if ( std::find( table_names.begin(), table_names.end(), std::string( name ) ) != table_names.end() ) {
            ADTRACE() << "sql.exec( DROP TABLE " << name << " )";
            sql.exec( std::string("DROP TABLE ") + name );
        }
    }
    sql.exec( "DELETE FROM file" );
    sql.exec( "DELETE from directory" );
    
    return true;
}

// static
bool
QuanDataWriter::insert_table( adfs::stmt& sql, const adcontrols::idAudit& t, const std::string& what )
{
    if ( sql.prepare("INSERT INTO idAudit (uuid,digest,dateCreated,idComputer,idCreatedBy,nameCreatedBy,reason) VALUES (?,?,?,?,?,?,?)" ) ) {
        
        sql.bind( 1 ) = boost::lexical_cast< std::string >( t.uuid() );
        sql.bind( 2 ) = std::string( t.digest() );
        sql.bind( 3 ) = std::string( t.dateCreated() );
        sql.bind( 4 ) = std::wstring( t.idComputer() );
        sql.bind( 5 ) = std::wstring( t.idCreatedBy() );
        sql.bind( 6 ) = std::wstring( t.nameCreatedBy() );
        sql.bind( 7 ) = what;

        return sql.step() == adfs::sqlite_done;
    }
    return false;
}

bool
QuanDataWriter::insert_table( const adcontrols::QuanMethod& t )
{
    adfs::stmt sql( fs_.db() );

    insert_table( sql, t.ident(), "Create QuanMethod" );

    if ( sql.prepare( "\
INSERT INTO QuanMethod (idAudit,uuid,equation,polynomialOrder,isChromatogram,isWeighting,isBracketing\
,bracketing,weighting,isISTD,levels,replicates,fnMethod,fnCompounds,fnSequence) \
SELECT idAudit.id,:uuid,?,?,?,?,?,?,?,?,?,?,?,?,? from idAudit WHERE uuid = :uuid" ) ) {
        int row = 1;
        sql.bind( row++ ) = boost::lexical_cast< std::string >( t.ident().uuid() ); // :uuid (idAudit.rowid)
        sql.bind( row++ ) = int64_t(t.equation());
        sql.bind( row++ ) = int64_t(t.polynomialOrder());
        sql.bind( row++ ) = int64_t(t.isChromatogram());
        sql.bind( row++ ) = int64_t(t.isWeighting());
        sql.bind( row++ ) = int64_t(t.isBracketing());
        sql.bind( row++ ) = int64_t(t.bracketing());
        sql.bind( row++ ) = int64_t(t.weighting());
        sql.bind( row++ ) = int64_t(t.isInternalStandard());
        sql.bind( row++ ) = t.levels();
        sql.bind( row++ ) = t.replicates();
        sql.bind( row++ ) = std::wstring( t.quanMethodFilename() );
        sql.bind( row++ ) = std::wstring( t.quanCompoundsFilename() );
        sql.bind( row++ ) = std::wstring( t.quanSequenceFilename() );

        if ( sql.step() != adfs::sqlite_done )
            ADTRACE() << "sql error";

    }
    return true;
}

bool
QuanDataWriter::insert_table( const adcontrols::QuanSequence& t )
{
    adfs::stmt sql( fs_.db() );

    sql.begin();

    insert_table( sql, t.ident(), "Create QuanSequence" );
    bool success = false;
    if ( sql.prepare( "INSERT INTO QuanSequence (idAudit,uuid,outfile) SELECT id, uuid, :outdata from idAudit WHERE uuid = :uuid" ) ) {

        sql.bind( 1 ) = std::wstring( t.outfile() );
        sql.bind( 2 ) = boost::lexical_cast<std::string>(t.uuid());
        
        if ( sql.step() == adfs::sqlite_done ) {
            success = true;
            sql.commit();
        }
    }
    if ( !success )
        sql.rollback();

    success = false;
    sql.begin();
    for ( auto& sample: t ) {

        if ( sql.prepare( "INSERT INTO QuanSample\
(idSequence,uuid,row,name,dataType,dataSource,dataGuid,sampleType,level,ISTDID,injVol,amountsAdded,channel,dataGeneration,data_first,data_second)\
SELECT id,uuid,?,?,?,?,?,?,?,?,?,?,?,?,?,? FROM QuanSequence WHERE uuid = :uuid" ) ) {
            int row = 1;
            sql.bind( row++ ) = sample.row();
            sql.bind( row++ ) = std::wstring( sample.name() );
            sql.bind( row++ ) = std::wstring( sample.dataType() );
            sql.bind( row++ ) = std::wstring( sample.dataSource() );
            sql.bind( row++ ) = std::wstring( sample.dataGuid() );
            sql.bind( row++ ) = int64_t( sample.sampleType() );
            sql.bind( row++ ) = sample.level();
            sql.bind( row++ ) = sample.istdId();
            sql.bind( row++ ) = sample.injVol();
            sql.bind( row++ ) = sample.addedAmounts();
            sql.bind( row++ ) = sample.channel();
            sql.bind( row++ ) = int64_t( sample.dataGeneration() );
            sql.bind( row++ ) = sample.scan_range_first();
            sql.bind( row++ ) = sample.scan_range_second();

            sql.bind( row++ ) = boost::lexical_cast< std::string >( t.uuid() ); // :uuid

            if ( sql.step() == adfs::sqlite_done )
                success = true;
            else {
                ADTRACE() << "sql error";
                break;
            }
        }
    }
    if ( success )
        sql.commit();

    return success;
}

bool
QuanDataWriter::insert_table( const adcontrols::QuanCompounds& t )
{
    adfs::stmt sql( fs_.db() );

    sql.begin();

    insert_table( sql, t.ident(), "Create QuanCompounds" );
    
    std::string uuid = boost::lexical_cast<std::string>(t.uuid());

    for ( auto& c: t ) {

        if ( sql.prepare( "INSERT INTO QuanCompound \
(uuid,uniqId,display_name,formula,idISTD,levels,mass,tR,isLKMSRef,isTimeRef,isISTD,description,criteria_0,criteria_1) \
VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?)" ) ) {
            int row = 1;
            sql.bind( row++ ) = uuid;
            sql.bind( row++ ) = c.uniqId();
            sql.bind( row++ ) = std::wstring( c.display_name() );
            sql.bind( row++ ) = std::string( c.formula() );        
            sql.bind( row++ ) = int64_t( c.idISTD() );
            sql.bind( row++ ) = int64_t( c.levels() );
            sql.bind( row++ ) = c.mass();
            sql.bind( row++ ) = c.tR();
            sql.bind( row++ ) = int64_t( c.isLKMSRef() );  // lock mass reference
            sql.bind( row++ ) = int64_t( c.isTimeRef() );  // relative retention reference
            sql.bind( row++ ) = int64_t( c.isISTD() );     // amount reference
            sql.bind( row++ ) = std::wstring( c.description() );
            sql.bind( row++ ) = c.criteria(0);
            sql.bind( row++ ) = c.criteria(1);

            if ( sql.step() != adfs::sqlite_done )
                ADTRACE() << "sql error";
        }

        for ( int i = 0; i < c.levels(); ++i ) {
            if ( sql.prepare( 
                     "\
INSERT INTO QuanAmount (idCompound, level, amount)  \
SELECT QuanCompound.id, :level, :amount \
FROM QuanCompound \
WHERE uuid = :uuid and uniqId = :uniqId" ) ) {
                sql.bind(1) = i + 1;             // :level (1-origin)
                sql.bind(2) = c.amounts()[i];    // :amount
                sql.bind(3) = uuid;              // QuanCompound.uuid
                sql.bind(4) = c.uniqId();        // QuanCompound.uniqId

                if ( sql.step() != adfs::sqlite_done )
                    ADTRACE() << "sql error";                    
            }
        }
    }
    sql.commit();
    return true;
}

bool
QuanDataWriter::insert_table( const adcontrols::QuanSample& t )
{
    adfs::stmt sql( fs_.db() );

    std::string uuidQuanSequence = boost::lexical_cast<std::string>(t.sequence_uuid());

    for ( auto& result: t.results() ) {
        if ( sql.prepare( "INSERT INTO QuanResponse \
(idSample,idx,fcn,uniqId,uniqGuid,intensity,formula,mass,tR) \
SELECT QuanSample.id,?,?,?,?,?,?,?,? \
FROM QuanSample WHERE QuanSample.idSequence = (SELECT QuanSequence.id FROM QuanSequence WHERE uuid = :uuid) AND QuanSample.row = :row" ) ) {
            int col = 1;
            sql.bind( col++ ) = result.idx_;
            sql.bind( col++ ) = result.fcn_;
            sql.bind( col++ ) = result.uniqId_;
            sql.bind( col++ ) = boost::lexical_cast<std::string>(result.uniqGuid_);
            sql.bind( col++ ) = result.intensity_;
            sql.bind( col++ ) = std::string( result.formula() );
            sql.bind( col++ ) = result.mass_;
            sql.bind( col++ ) = result.tR_;
            sql.bind( col++ ) = uuidQuanSequence;    // QuanSequence.uuid
            sql.bind( col++ ) = uint64_t( t.row() );            // QuanSequence.uuid

            if ( sql.step() != adfs::sqlite_done )
                ADERROR() << "sql error";
        }
    }
    return true;
}
