/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "quanchromatograms.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/massspectra.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/quansample.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adfs/adfs.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <adfs/cpio.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportable/profile.hpp>
#include <adportable/uuid.hpp>
#include <adportable/utf.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <numeric>

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

void
QuanDataWriter::remove( const std::wstring& title, const wchar_t * directory )
{
    if ( adfs::folder folder = fs_.addFolder( directory ) ) { // directory := L"/Processed/Spectra" | L"/Processed/Chromatograms" ...

        auto vec = folder.files();
        std::for_each( vec.begin(), vec.end(), [=] ( const adfs::file& f ) {
            if ( f.attribute( L"name" ) == title ) {

                adfs::stmt sql( fs_.db() );
                sql.begin();

                auto atts = f.attachments();
                atts.push_back( f );  // append 'file' at the end if attachements

                for ( auto att : atts ) {
                    auto guid = att.name();
                    sql.prepare( "DELETE FROM file WHERE fileid = (SELECT fileid FROM directory WHERE name = ?)" );
                    sql.bind( 1 ) = adportable::utf::to_utf8( guid );
                    if ( sql.step() == adfs::sqlite_done ) {
                        sql.prepare( "DELETE FROM directory WHERE name = ?" );
                        sql.bind( 1 ) = adportable::utf::to_utf8( guid );
                        sql.step();
                    }
                }
                sql.commit();
            }
        } );
    }
}

adfs::file
QuanDataWriter::write( const adcontrols::MassSpectrum& ms, const std::wstring& tittle )
{
    if ( adfs::folder folder = fs_.addFolder( L"/Processed/Spectra" ) ) {
        if ( adfs::file file = folder.addFile( adfs::create_uuid(), tittle ) ) {
            file.dataClass( ms.dataClass() );
            if ( file.save( ms ) )
                file.commit();
            return file;
        }
    }
    return adfs::file();
}

adfs::file
QuanDataWriter::write( const adcontrols::MassSpectra& a, const std::wstring& tittle )
{
    if ( adfs::folder folder = fs_.addFolder( L"/Processed/Spectrograms" ) ) {
        if ( adfs::file file = folder.addFile( adfs::create_uuid(), tittle ) ) {
            file.dataClass( a.dataClass() );
            if ( file.save( a ) )
                file.commit();
            return file;
        }
    }
    return adfs::file();
}

adfs::file
QuanDataWriter::write( const adcontrols::Chromatogram& c, const std::wstring& tittle )
{
    if ( adfs::folder folder = fs_.addFolder( L"/Processed/Chromatograms" ) ) {

        if ( adfs::file file = folder.addFile( adfs::create_uuid(), tittle ) ) {

            file.dataClass( c.dataClass() );

            if ( file.save( c ) )
                file.commit();

            return file;
        }
    }
    return adfs::file();
}

adfs::file
QuanDataWriter::write( const adcontrols::Chromatogram& c, const wchar_t * dataSource, const std::wstring& title )
{
    if ( auto top = fs_.addFolder( L"/Processed/Chromatograms" ) ) {

        boost::filesystem::path path = boost::filesystem::path( L"/Processed/Chromatograms" ) / boost::filesystem::path( dataSource ).stem();
        ADDEBUG() << path.string();

        if ( adfs::folder folder = fs_.addFolder( path.wstring() ) ) {
            if ( adfs::file file = folder.addFile( adfs::create_uuid(), title ) ) {
                file.dataClass( c.dataClass() );
                if ( file.save( c ) )
                    file.commit();
                return file;
            }
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
            if ( file.save( pm ) )
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
            if ( file.save( t ) )
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
,uuid          UUID \
,digest        TEXT \
,dateCreated   DATE \
,idComputer    TEXT \
,idCreatedBy   TEXT \
,nameCreatedBy TEXT \
,reason        TEXT\
,UNIQUE(uuid) )" );

    // MetaData, see acquireddata_v3.cpp L88
    result &= sql.exec( "CREATE TABLE IF NOT EXISTS "
                        "MetaData ( clsid UUID, attrib TEXT, data BLOB )"  );

    result &= sql.exec(
        "CREATE TABLE QuanMethod (\
 id      INTEGER PRIMARY KEY \
,idAudit         INTEGER \
,uuid            UUID \
,equation        INTEGER \
,polynomialOrder INTEGER \
,isCounting      INTEGER \
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
,uuid    UUID \
,outfile TEXT \
,UNIQUE(uuid) \
,FOREIGN KEY ( idAudit ) REFERENCES idAudit ( id ) )" );

// QuanSample is corresponding to a sample, which is consisted from several compounds.
// It is equivalent to a chromatogram, or a spectrum.  Each peak detected from them are stroing into QuanResponse

    result &= sql.exec(
        "CREATE TABLE IF NOT EXISTS QuanSample (\
 id INTEGER PRIMARY KEY \
,uuid            UUID \
,idSequence      INTEGER \
,uidQuanSequence UUID    \
,row             INTEGER \
,name            TEXT    \
,dataType        TEXT    \
,dataSource      TEXT    \
,sampleType      INTEGER \
,level           INTEGAR \
,ISTDID          INTEGAR \
,injVol          REAL    \
,amountsAdded    REAL    \
,channel         INTAGER \
,dataGeneration  INTEGER \
,data_first      INTEGER \
,data_second     INTEGER \
,UNIQUE(uuid) \
,UNIQUE(idSequence,row)  \
,FOREIGN KEY( uidQuanSequence ) REFERENCES QuanSequence( uuid ))" );

    result &= sql.exec(
        "CREATE TABLE QuanISTD (\
 idSample INTEGER \
,ISTDID  INTEGAR \
,amounts REAL    \
,FOREIGN KEY( idSample ) REFERENCES QuanSample( id ) \
)" );


// QuanCompound := table of compounds so that all compound in a table share uuid
// idCmpd is unique within a table, but not unique across the tables
// 'id' is actual unique key within a database

    result &= sql.exec(
        "CREATE TABLE QuanCompound (\
 id  INTEGER PRIMARY KEY \
,uuid           UUID     \
,idTable        UUID     \
,row            INTEGER  \
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
,isCounting     INTEGER  \
,UNIQUE(uuid) \
,FOREIGN KEY( idTable ) REFERENCES idAudit( uuid ))" );

    result &= sql.exec(
        "CREATE TABLE QuanAmount (\
 idCompound INTEGER \
,idCmpd UUID \
,level INTEGER \
,amount REAL \
,FOREIGN KEY( idCmpd ) REFERENCES QuanCompound ( uuid ) \
,FOREIGN KEY( idCompound ) REFERENCES QuanCompound ( id ) )" );

    result &= sql.exec(
        "CREATE TABLE QuanResponse ( \
 id     INTEGER PRIMARY KEY \
,idSample       INTEGAR \
,idx            INTEGER \
,fcn            INTEGAR \
,intensity      REAL    \
,idCmpd         UUID    \
,idTable        UUID    \
,dataGuid       TEXT    \
,formula        TEXT    \
,mass           REAL    \
,tR             REAL    \
,calibId        INTEGER \
,amount         REAL    \
,timeCounts     INTEGER \
,trigCounts     INTEGER \
,FOREIGN KEY( idSample ) REFERENCES QuanSample ( id ) )" );

    result &= sql.exec("\
CREATE TABLE QuanCalib (\
 id INTEGER PRIMARY KEY\
,uuid          UUID    \
,idCompound    INTEGER \
,idTable       UUID \
,idCmpd        UUID \
,idMethod      UUID \
,n             INTEGER \
,min_x         REAL \
,max_x         REAL \
,chisqr        REAL \
,a REAL, b REAL, c REAL, d REAL, e REAL, f REAL \
,date       TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL \
,FOREIGN KEY ( idCompound ) REFERENCES QuanCompound ( id ) \
,FOREIGN KEY ( uuid ) REFERENCES idAudit ( uuid ) \
)" );

    result &= sql.exec("CREATE TABLE QuanDataGuids( dataGuid TEXT, refDataGuid TEXT, idx INTEGER, fcn INTEGER )" );

    result &= sql.exec(
"CREATE TABLE cpeak ("
" id INTEGER PRIMARY KEY "
",dataGuid      UUID "  // <- unique with directory.name
",name          TEXT "
",formula       TEXT "
",startTime     REAL "
",peakTime      REAL "
",endTime       REAL "
",startHeight   REAL "
",topHeight     REAL "
",endHeight     REAL "
",peakArea      REAL "
",peakHeight    REAL "
",asymmetry     REAL "
",resolution    REAL "
",ntp           REAL "
",capacityfactor REAL "
",peakwidth     REAL "
",tR            REAL "
",tR_algo       INTEGER "
",tR_a REAL, tR_b REAL, tR_c REAL"
//",FOREIGN KEY ( parent_id, dataGuid ) REFERENCES directory ( parent_id, name ) "
")" );

    return result;
}

bool
QuanDataWriter::create_counting_tables()
{
    adfs::sqlite & db = fs_.db();

    adfs::stmt sql( db );
    bool result( true );

    result &= sql.exec(
        "CREATE TABLE QuanCountingResponse ("
        " id     INTEGER PRIMARY KEY"
        ",idSample       INTEGAR"
        ",dataSource     TEXT"
        ",fcn            INTEGAR"
        ",idCmpd         UUID"
        ",dataGuid       UUID"
        ",formula        TEXT"
        ",response       REAL"
        ",tof            REAL"
        ",stddev         REAL"
        ",N              INTEGER"
        ",centroid       TEXT"
        ",FOREIGN KEY( idSample ) REFERENCES QuanSample ( id ) )" );

    result &= sql.exec(
        "CREATE TABLE QuanCountingData ("
        "idSample       INTEGAR"
        ",idCmpd        INTEGAR"
        ",fcn           INTEGAR"
        ",idx           INTEGAR"
        ",time          REAL"
        ",response      REAL"
        ",tof           REAL"
        ",mass          REAL"
        ",stem          TEXT"
        ",FOREIGN KEY( idSample ) REFERENCES QuanSample ( id ) )" );

    result &= sql.exec(
        "CREATE TABLE MSComp ("
        "idSample       INTEGAR"
        ",epochTime     INTEGER"
        ",a             REAL"
        ",b             REAL"
        ",FOREIGN KEY( idSample ) REFERENCES QuanSample ( id ) )" );

    return result;
}

bool
QuanDataWriter::drop_table()
{
    adfs::stmt sql( fs_.db() );

    static const char * drop_order[] = {
        "group"
        , "QuanResponse"
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

        sql.bind( 1 ) = t.uuid();
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
INSERT INTO QuanMethod (idAudit,uuid,equation,polynomialOrder,isCounting,isChromatogram,isWeighting,isBracketing\
,bracketing,weighting,isISTD,levels,replicates,fnMethod,fnCompounds,fnSequence) \
SELECT idAudit.id,:uuid,?,?,?,?,?,?,?,?,?,?,?,?,?,? from idAudit WHERE uuid = :uuid" ) ) {

        int row = 1;
        sql.bind( row++ ) = t.ident().uuid(); // :uuid (idAudit.rowid)
        sql.bind( row++ ) = int64_t(t.equation());
        sql.bind( row++ ) = int64_t(t.polynomialOrder());
        sql.bind( row++ ) = int64_t(t.isCounting());
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
        sql.bind( 2 ) = t.uuid();

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

        if ( sql.prepare( "INSERT INTO QuanSample"
                          "(uuid"
                          ",idSequence"
                          ",uidQuanSequence"
                          ",row"
                          ",name"
                          ",dataType"
                          ",dataSource"
                          ",sampleType"
                          ",level"
                          ",ISTDID"
                          ",injVol"
                          ",amountsAdded"
                          ",channel"
                          ",dataGeneration"
                          ",data_first"
                          ",data_second)"
                          "SELECT ?,id,uuid,?,?,?,?,?,?,?,?,?,?,?,?,? FROM QuanSequence WHERE uuid = :uuid" ) ) {

            int row = 1;
            sql.bind( row++ ) = sample.uuid();  // own unique id
            // <-- QuanSequence.id where QuanSequence.uuid = t.uuid // parent (sequence) id
            sql.bind( row++ ) = sample.row();                   // parent row#
            sql.bind( row++ ) = std::wstring( sample.name() );
            sql.bind( row++ ) = std::wstring( sample.dataType() );
            sql.bind( row++ ) = std::wstring( sample.dataSource() );
            // sql.bind( row++ ) = std::wstring( sample.dataGuid() );
            sql.bind( row++ ) = int64_t( sample.sampleType() );
            sql.bind( row++ ) = sample.level();
            sql.bind( row++ ) = sample.istdId();
            if ( sample.inletType() == adcontrols::Quan::Chromatography )
                sql.bind( row++ ) = sample.injVol();
            else
                sql.bind( row++ ) = adfs::null();

            sql.bind( row++ ) = sample.addedAmounts();
            sql.bind( row++ ) = sample.channel();
            sql.bind( row++ ) = int64_t( sample.dataGeneration() );
            sql.bind( row++ ) = sample.scan_range_first();
            sql.bind( row++ ) = sample.scan_range_second();

            sql.bind( row++ ) = t.uuid(); // QuanSequence.uuid (uidQuanSequence)

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

    insert_table( sql, t.ident(), "Create QuanCompounds" );

    sql.begin();
    for ( auto& c: t ) {

        if ( sql.prepare( "INSERT INTO QuanCompound \
(uuid,idTable,row,display_name,formula,idISTD,levels,mass,tR,isLKMSRef,isTimeRef,isISTD,description,criteria_0,criteria_1,isCounting) \
VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)" ) ) {

            int row = 1;
            sql.bind( row++ ) = c.uuid(); // uuid reference as 'idCmpd'
            sql.bind( row++ ) = t.uuid(); // idTable
            sql.bind( row++ ) = c.row();  // row
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
            sql.bind( row++ ) = int64_t( c.isCounting() );

            if ( sql.step() != adfs::sqlite_done ) {
                ADTRACE() << "sql error";
                return false;
            }

        }

        for ( int i = 0; i < int(c.levels()); ++i ) {
            if ( sql.prepare(
                     "INSERT INTO QuanAmount (idCompound, idCmpd, level, amount)"
                     "SELECT QuanCompound.id, :idCmpd, :level, :amount "
                     "FROM QuanCompound WHERE uuid = :idCmpd" ) ) {
                int row = 1;
                sql.bind( row++ ) = c.uuid();          // QuanCompound.idCmpd
                sql.bind( row++ ) = i + 1;             // :level (1-origin)
                sql.bind( row++ ) = c.amounts()[ i ];  // :amount

                if ( sql.step() != adfs::sqlite_done ) {
                    ADTRACE() << "sql error";
                    return false;
                }

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

    sql.begin();

    // ADDEBUG() << "inserting QuanResponse table size=" << t.results().size();

    for ( auto& result: t.results() ) {

        if ( sql.prepare( "INSERT INTO QuanResponse"
                          "(idSample,idx,fcn,intensity,idCmpd,idTable,dataGuid,formula,mass,tR,timeCounts,trigCounts)"
                          "SELECT QuanSample.id,?,?,?,?,?,?,?,?,?,?,?"
                          "FROM QuanSample WHERE QuanSample.uuid = :uuid") ) {

            // ADDEBUG() << "insert_table fcn: " << result.fcn_ << ", " << result.uuid_cmpd();

            int col = 1;
            sql.bind( col++ ) = result.idx_;
            sql.bind( col++ ) = result.fcn_;
            sql.bind( col++ ) = result.intensity();
            sql.bind( col++ ) = result.uuid_cmpd();              // QuanCompound.uuid
            sql.bind( col++ ) = result.uuid_cmpd_table();        // QuanCompounds.uuid (idTable)
            sql.bind( col++ ) = result.dataGuid();                // QuanCompounds.uuid (idTable)
            sql.bind( col++ ) = std::string( result.formula() ); // identified formula (not equal to formula on compound if targeting applied)
            sql.bind( col++ ) = result.mass();                    // obserbed mass
            sql.bind( col++ ) = result.tR();                      // observed retention time
            sql.bind( col++ ) = result.countTimeCounts();        // ion count
            sql.bind( col++ ) = result.countTriggers();          // total trigger count
            sql.bind( col++ ) = t.uuid();                        // QuanSample.uuid

            if ( sql.step() != adfs::sqlite_done )
                ADTRACE() << "sql error";
        }
    }
    sql.commit();
    return true;
}

bool
QuanDataWriter::insert_reference( const boost::uuids::uuid& dataGuid, const boost::uuids::uuid& refGuid, int32_t idx, int32_t proto )
{
    adfs::stmt sql( fs_.db() );

    if ( sql.prepare( "INSERT INTO QuanDataGuids (dataGuid,refDataGuid,idx,fcn) VALUES (?,?,?,?)" ) ) {

        sql.bind( 1 ) = dataGuid;
        sql.bind( 2 ) = refGuid;
        sql.bind( 3 ) = idx;
        sql.bind( 4 ) = proto;

        if ( sql.step() == adfs::sqlite_done )
            return true;

        ADTRACE() << "sql error";
    }
    return false;
}

bool
QuanDataWriter::insert_table( const std::wstring& dataGuid, const std::vector< std::tuple<std::wstring, uint32_t, uint32_t > >& dataGuids )
{
    if ( !dataGuid.empty() ) {

        adfs::stmt sql( fs_.db() );

        for ( const auto& guid: dataGuids ) {
            if ( !std::get<0>( guid ).empty() ) {
                if ( sql.prepare( "INSERT INTO QuanDataGuids (dataGuid,refDataGuid,idx,fcn) VALUES (?,?,?,?)" ) ) {

                    sql.bind( 1 ) = dataGuid;
                    sql.bind( 2 ) = std::get<0>( guid );
                    sql.bind( 3 ) = std::get<1>( guid ); // idx
                    sql.bind( 4 ) = std::get<2>( guid ); // fcn

                    if ( sql.step() != adfs::sqlite_done )
                        ADTRACE() << "sql error";
                }
            }
        }

        return true;
    }
    return false;
}

bool
QuanDataWriter::addCountingResponse( const boost::uuids::uuid& dataGuid // chromatogram file id
                                     , const adcontrols::QuanSample& sample
                                     , const adcontrols::Chromatogram& chro )
{
    double mean(0), variance(0);
    size_t N = chro.size();

    if ( chro.size() )
        mean = std::accumulate( chro.getIntensityArray(), chro.getIntensityArray() + N, 0.0 ) / N;

    if ( chro.size() > 1 )
        variance = std::accumulate( chro.getIntensityArray(), chro.getIntensityArray() + N, 0.0
                                    , [&](const auto& a, const auto& v){ return a + (v - mean) * (v - mean); }) / (N - 1);

    // ADDEBUG() << "addCountingResponse( proto: " << chro.protocol() << ", mean: " << mean << ")";

    if ( auto child = chro.ptree().get_child_optional( "generator.extract_by_mols" ) ) {
        if ( auto cmpdGuid = child.get().get_optional< boost::uuids::uuid >( "molid" ) ) { // "generator.extract_by_mols.molid"
            if ( auto mol = child.get().get_child_optional( "moltable" ) ) {               // "generator.extract_by_mols.moltable"
                auto formula = mol.get().get_optional< std::string >( "formula" );
                auto proto = mol.get().get_optional< int32_t >( "protocol" );
                if ( formula && proto ) {

                    //ADDEBUG() << " " << cmpdGuid.get() << ", formula: "
                    //<< formula.get() << ", proto: " << proto.get() << ", average: " << resp << ", idSample: " << sample.row();
                    auto tof = child.get().get_optional<double>( "tof" );
                    auto centroid = child.get().get_optional< std::string >( "centroid" );

                    adfs::stmt sql( fs_.db() );

                    if ( sql.prepare(
                             "INSERT INTO QuanCountingResponse (idSample,dataSource,fcn,idCmpd,dataGuid,formula,response,tof,stddev,N,centroid)"
                             " VALUES ((SELECT id FROM QuanSample WHERE uuid = ?),?,?,?,?,?,?,?,?,?,?)" ) ) {
                        sql.bind( 1 ) = sample.uuid();
                        sql.bind( 2 ) = boost::filesystem::path( sample.dataSource() ).stem().string();
                        sql.bind( 3 ) = proto.get();
                        sql.bind( 4 ) = cmpdGuid.get();
                        sql.bind( 5 ) = dataGuid;
                        sql.bind( 6 ) = formula.get();
                        sql.bind( 7 ) = mean;
                        sql.bind( 8 ) = ( tof ? tof.get() : 0 );
                        sql.bind( 9 ) = std::sqrt( variance );
                        sql.bind( 10 ) = N;
                        sql.bind( 11 ) = ( centroid ? centroid.get() : "" );
                        if ( sql.step() != adfs::sqlite_done ) {
                            ADTRACE() << "sql error " << sql.errmsg();
                            return false;
                        }
                    }
                } else {
                    ADDEBUG() << "## Error: formula/proto not found";
                }

                auto tof = child.get().get_optional<double>( "tof" );
                adfs::stmt sql( fs_.db() );
                if ( sql.prepare( "INSERT INTO QuanCountingData (idSample,idCmpd,fcn,idx,time,response,tof,mass) VALUES"
                                  "((SELECT id FROM QuanSample WHERE uuid = ?)"
                                  ",(SELECT id FROM QuanCompound WHERE uuid = ?)"
                                  ",?,?,?,?,?,?)" ) ) {
                    for ( size_t i = 0; i < N; ++i ) {
                        sql.bind( 1 ) = sample.uuid();
                        sql.bind( 2 ) = cmpdGuid.get();
                        sql.bind( 3 ) = chro.protocol();
                        sql.bind( 4 ) = i;
                        sql.bind( 5 ) = chro.time( i );
                        sql.bind( 6 ) = chro.intensity( i );
                        sql.bind( 7 ) = chro.tofArray().empty() ? tof.get() : chro.tof( i );
                        sql.bind( 8 ) = chro.mass( i );
                        // sql.bind( 9 ) = boost::filesystem::path( sample.dataSource() ).stem().string();
                        if ( sql.step() != adfs::sqlite_done )
                            ADTRACE() << "sql error " << sql.errmsg();
                        sql.reset();
                    }
                }

            } else {
                ADDEBUG() << "## Error: moltable not found";
                return false;
            }
        } else {
            ADDEBUG() << "## Error: molid not found";
            return false;
        }
    }
    return true;
}


bool
QuanDataWriter::addMSLock( const adcontrols::QuanSample& sample
                           , const std::vector< std::pair< int64_t, std::array< double, 2 > > >& lkms ) // time, coeffs
{
    if ( lkms.empty() )
        return true;

    adfs::stmt sql( fs_.db() );
    sql.begin();
    if ( sql.prepare( "INSERT INTO MSComp (idSample,epochTime,a,b) VALUES ((SELECT id FROM QuanSample WHERE uuid=?),?,?,?)" ) ) {
        for ( const auto& pair : lkms ) {
            sql.bind( 1 ) = sample.uuid();
            sql.bind( 2 ) = pair.first; // time
            sql.bind( 3 ) = pair.second[0];
            sql.bind( 4 ) = pair.second[1];
            if ( sql.step() != adfs::sqlite_done )
                ADTRACE() << "sql error " << sql.errmsg();
            sql.reset();
        }
    }
    sql.commit();

    return true;
}

bool
QuanDataWriter::addMSLock( std::shared_ptr< adprocessor::dataprocessor> dp
                           , const std::vector< std::pair< int64_t, std::array< double, 2 > > >& lkms )
{
    if ( lkms.empty() )
        return true;

    if ( auto db = dp->db() ) {
        adfs::stmt sql( *db );
        sql.exec( "DROP TABLE IF EXISTS MSComp" );
        sql.exec( "CREATE TABLE MSComp (epochTime INTEGER, a REAL, b REAL)" );

        sql.begin();
        if ( sql.prepare( "INSERT INTO MSComp (epochTime,a,b) VALUES (?,?,?)" ) ) {
            for ( const auto& pair : lkms ) {
                sql.bind( 1 ) = pair.first;
                sql.bind( 2 ) = pair.second[0];
                sql.bind( 3 ) = pair.second[1];
                if ( sql.step() != adfs::sqlite_done )
                    ADTRACE() << "sql error " << sql.errmsg();
                sql.reset();
            }
        }
        sql.commit();
    }
    return true;
}

bool
QuanDataWriter::create_spectrogram_tables()
{
    adfs::sqlite & db = fs_.db();

    adfs::stmt sql( db );
    bool result( true );

    result &= sql.exec(
        "CREATE TABLE SGSpectrum ("
        "idSample       INTEGAR"
        ",pkd           INTEGAR"
        ",epochTime     INTEGAR"
        ",injTime       REAL"
        ",trigNumber    INTEGER"
        ",stem          TEXT"
        ",FOREIGN KEY( idSample ) REFERENCES QuanSample ( id ) )" );

    result &= sql.exec(
        "CREATE TABLE SGPeak ("
        "idSample       INTEGAR"
        ",pkd           INTEGER"
        ",fcn           INTEGER"
        ",mass          REAL"
        ",time          REAL"
        ",intensity     REAL"
        ",FOREIGN KEY( idSample ) REFERENCES QuanSample ( id ) )" );

    return result;
}

bool
QuanDataWriter::insert_spectrogram( const boost::uuids::uuid& fileGuid
                                    , adcontrols::MassSpectra& a
                                    , const adprocessor::dataprocessor& dp
                                    , int idx ) // 0 = AVG, 1 = PKD
{
    ADDEBUG() << dp.filename();

    uint64_t id(0);
    adfs::stmt sql( fs_.db() );
    sql.prepare( "SELECT id FROM QuanSample WHERE dataSource like ?" );
    sql.bind(1) = dp.filename();
    if ( sql.step() == adfs::sqlite_row )
        id = sql.get_column_value< uint64_t >(0);
    else
        return false;

    for ( const auto& ms: a ) {
        const auto& prop = ms->getMSProperty();
        uint64_t epoch_time = prop.timeSinceEpoch();
        double inj_time = prop.timeSinceInjection();
        uint32_t trigNumber = prop.trigNumber( false );

        sql.prepare( "INSERT INTO SGSpectrum ( idSample, pkd, epochTime, injTime, trigNumber ) VALUES (?,?,?,?,?)" );
        sql.bind( 1 ) = id;
        sql.bind( 2 ) = idx; // AVG == 0, PKD == 1
        sql.bind( 3 ) = epoch_time;
        sql.bind( 4 ) = inj_time;
        sql.bind( 4 ) = trigNumber;

        if ( sql.step() == adfs::sqlite_done ) {

            //sql.begin();
            sql.prepare( "INSERT INTO SGPeak ( idSample, pkd, mass, time, intensity ) VALUES (?,?,?,?,?)" );
            for ( size_t i = 0; i < ms->size(); ++i ) {
                sql.bind( 1 ) = id;
                sql.bind( 2 ) = idx;
                sql.bind( 3 ) = ms->mass( i );
                sql.bind( 4 ) = ms->time( i );
                sql.bind( 5 ) = ms->intensity( i );
                if ( sql.step() != adfs::sqlite_done )
                    ADTRACE() << "sql error " << sql.errmsg();
                sql.reset();
            }
            //sql.commit();
        }
    }
    return true;
}
