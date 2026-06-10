/**************************************************************************
** Copyright (C) 2022-2026 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2026 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "constants.hpp"
#include "document.hpp"
#include "sdfimport.hpp"
#include "bzip2_helper.hpp"
#include <adchem/sdfile.hpp>
#include <adchem/sdmol.hpp>
#include <adchem/sdmolsupplier.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <adportable/bzip2.hpp>
#include <adwidgets/progressinterface.hpp>
#include <qtwrapper/settings.hpp>
#include <coreplugin/progressmanager/progressmanager.h>
#include <RDGeneral/Invariant.h>
#include <GraphMol/Depictor/RDDepictor.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <GraphMol/inchi.h>
#include <QCoreApplication>
#include <QFileDialog>
#include <boost/json.hpp>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <future>
#include <thread>

using lipidid::SDFileImport;

namespace lipidid {

    class SDFileImport::impl {
    public:
        typedef std::tuple< RDKit::ROMol
                            , std::string // inchi
                            , std::vector< std::pair< std::string, std::string > > // dataItems
                            > value_type;

        std::vector< value_type > values_;
        std::shared_ptr< adchem::SDFile > sdfile_; // keep it

        void populate( std::shared_ptr< adchem::SDFile > sdfile
                       , std::shared_ptr< adfs::sqlite > sqlite
                       , std::shared_ptr< adwidgets::ProgressInterface > p );

        static impl& instance() {
            static impl __instance;
            return __instance;
        }
    private:
        void task(  std::shared_ptr< adchem::SDFile > sdfile
                    , std::shared_ptr< adfs::sqlite > sqlite
                    , std::shared_ptr< adwidgets::ProgressInterface > p );
        void add_duplicated( adfs::sqlite& db
                             , const RDKit::RWMol * mol
                             , const std::string& inchiKey
                             , const std::string& inchi
                             , const std::string& smiles
                             , const std::string& formula
                             , double mass
                             , double logP
                             , const adchem::SDMol& sdmol ) const;
    };

    struct SlogP {
        std::optional< double > operator()( const std::vector< std::pair< std::string, std::string > >& itemData ) const {
            auto it = std::find_if( itemData.begin(), itemData.end()
                                    , [](const auto& a){ return a.first == "SlogP"; });
            if ( it == itemData.end() )
                return {};
            else {
                char * end;
                return std::strtod( it->second.c_str(), &end );
            }
        }
    };

    struct itemSelector {

        std::string operator()( const std::vector< std::pair< std::string, std::string > >& list, const std::string& key ) const {
            auto it = std::find_if( list.begin(), list.end(), [&](const auto& a){ return a.first == key; } );
            if ( it != list.end() )
                return it->second;
            return {};
        }

        std::string to_value( const std::vector< std::pair< std::string, std::string > >& list ) const {
            return boost::json::serialize( boost::json::value{{"dataItem", boost::json::value_from( list )}} );
        }

    };

}

SDFileImport::~SDFileImport()
{
    ADDEBUG() << "### SDFileImport DTOR ###";
}

SDFileImport::SDFileImport( QWidget * parent ) : QWidget( parent )
{
}

bool
SDFileImport::import()
{
    QString fn = QFileDialog::getOpenFileName(
        nullptr
        , QObject::tr("Open File...")
        , qtwrapper::settings( *document::settings() ).recentFile( "SDF", "Files" )
        , QObject::tr("SDF Files (*.sdf);;All Files (*)"));

    if ( !fn.isEmpty() ) {
        auto path = std::filesystem::path( fn.toStdString() );
        std::error_code ec;
        if ( std::filesystem::exists( path, ec ) ) {
            qtwrapper::settings( *document::settings() ).addRecentFiles( "SDF", "Files", fn );
            auto sdfile = adchem::SDFile::create( path.string() ); // std::shared_ptr< SDFile >

            if ( auto sqlite = create_tables( path.stem().string() ) ) {
                if ( *sdfile ) {
                    auto p = std::make_shared< adwidgets::ProgressInterface >();
                    impl::instance().populate( sdfile, sqlite, p );
                }
            }
        }
    }
    return false;
}

std::shared_ptr< adfs::sqlite >
SDFileImport::create_tables( const std::string& stem )
{
    using qtwrapper::settings;
    std::filesystem::path fpath( settings( *document::settings() ).recentFile( "LIPID_MAPS", "Files" ).toStdString() );
    if ( fpath.empty() ) {
        auto path = std::filesystem::path( document::settings()->fileName().toStdString() );
        auto dir = path.remove_filename() / "lipidid";
        fpath = (dir / stem).replace_extension( ".db" );
    } else {
        auto dir = fpath.remove_filename();
        fpath = (dir / stem).replace_extension( ".db" );
    }

    QString fn = QFileDialog::getSaveFileName(
        nullptr
        , QObject::tr("Save File...")
        , QString::fromStdString( fpath.string() )
        , QObject::tr("SQLite Files (*.db *.sqlite);;All Files (*)") );

    if ( !fn.isEmpty() ) {
        auto file = std::filesystem::path( fn.toStdString() );
        if ( std::filesystem::exists( file ) ) {
            auto ofile = file.string() + std::string( ".bak" );
            if ( std::filesystem::exists( ofile ) )
                std::filesystem::remove( ofile );
            std::filesystem::rename( file, ofile );
        }
        if ( auto sqlite = std::make_shared< adfs::sqlite >() ) {
            if ( sqlite->open( file.string().c_str(), adfs::opencreate ) ) {
                auto sql = adfs::stmt( *sqlite );
                sql.exec(
                    "CREATE TABLE IF NOT EXISTS mols ("
                    "id INTEGER PRIMARY KEY"
                    ",inchiKey         TEXT"
                    ",smiles           TEXT" //
                    ",formula          TEXT"
                    ",mass             REAL"
                    ",SlogP            REAL"
                    ",svg              BLOB"
                    ",lm_id            TEXT"
                    ",itemData         TEXT" //  JSON
                    ",inchi            TEXT"
                    ",lm_ctab          BLOB"
                    ",UNIQUE(inchiKey)"
                    ")"
                    );
                sql.exec(
                    "CREATE TABLE IF NOT EXISTS duplicated_mols ("
                    "id INTEGER PRIMARY KEY"
                    ",inchiKey         TEXT"
                    ",smiles           TEXT" //
                    ",formula          TEXT"
                    ",mass             REAL"
                    ",SlogP            REAL"
                    ",svg              BLOB"
                    ",itemData         TEXT" //  JSON
                    ",lm_id            TEXT"
                    ",inchi            TEXT"
                    ",lm_ctab          BLOB"
                    //",UNIQUE(inchiKey)"
                    ")"
                    );
                qtwrapper::settings( *document::settings() ).addRecentFiles( "LIPID_MAPS", "Files", fn );
            }
            return sqlite;
        }
    }
    return {};
}

void
SDFileImport::impl::populate( std::shared_ptr< adchem::SDFile > sdfile
                              , std::shared_ptr< adfs::sqlite > sqlite
                              , std::shared_ptr< adwidgets::ProgressInterface > p )
{
    sdfile_ = sdfile;
    Core::ProgressManager::addTask( p->progress.future(), "Processing...", Constants::LIPIDID_TASK_SDFIMPORT );

    (*p)( 0, sdfile->size() );

    auto future = std::async( std::launch::async, [=,this](){ task( sdfile, sqlite, p ); } );

    while ( std::future_status::ready != future.wait_for( std::chrono::milliseconds( 100 ) ) )
        QCoreApplication::instance()->processEvents();
    future.wait();
}

void
SDFileImport::impl::task( std::shared_ptr< adchem::SDFile > sdfile
                          , std::shared_ptr< adfs::sqlite > sqlite
                          , std::shared_ptr< adwidgets::ProgressInterface > progress )
{
    adfs::stmt sql( *sqlite );

    // auto bzip2_compress = [](const std::string& svg)->adfs::blob{
    //     std::string bz2;
    //     adportable::bzip2::compress( bz2, svg.data(), svg.size() );
    //     return adfs::blob( bz2.size(), bz2.data() );
    // };

    std::vector< size_t > duplicated;
    std::pair< size_t, size_t > error_c{0,0};

    if ( sql.prepare("INSERT INTO mols ("
                     " id"                // 1
                     ",inchiKey"          // 2
                     ",smiles"            // 3
                     ",formula"           // 4
                     ",mass"              // 5
                     ",SlogP"             // 6
                     ",itemData"          // 7
                     ",lm_id"             // 8
                     ",inchi"             // 9
                     ",lm_ctab"           // 10
                     ",svg"               // 11
                     ") VALUES (?,?,?,?,?,?,?,?,?,?,?)") ) {

        for ( size_t i = 0; i < sdfile->size(); ++i ) {
            auto sdmol     = sdfile->at( i );
            auto ctab = sdmol.ctable();
            auto lm_id = itemSelector()( sdmol.dataItems(), "LM_ID" );

            if ( ( i % 1000 ) == 0 ) {
                ADDEBUG() << i << "/" << sdfile->size() << "\t" << std::make_tuple( i, lm_id );
            }

            if ( not ctab.empty() ) {
                std::unique_ptr< RDKit::RWMol > mol( RDKit::MolBlockToMol( ctab ) );
                auto inchiKey = RDKit::MolToInchiKey( *mol );
                if ( not inchiKey.empty() ) {
                    RDKit::ExtraInchiReturnValues rv;
                    auto inchi = RDKit::MolToInchi( *mol, rv );
                    auto smiles = RDKit::MolToSmiles( *mol );
                    auto json = itemSelector().to_value( sdmol.dataItems() );
                    double mass = RDKit::Descriptors::calcExactMW( *mol );
                    double logP, mr;
                    RDKit::Descriptors::calcCrippenDescriptors( *mol, logP, mr );
                    auto formula = RDKit::Descriptors::calcMolFormula( *mol, true, false );
                    auto svgz =  bzip2_compress()( sdmol.svg() );
                    auto ctabz = bzip2_compress()( ctab );

                    sql.bind( 1 ) = sdmol.index();
                    sql.bind( 2 ) = inchiKey;
                    sql.bind( 3 ) = smiles;
                    sql.bind( 4 ) = formula;
                    sql.bind( 5 ) = mass;
                    sql.bind( 6 ) = logP;
                    sql.bind( 7 ) = json;
                    sql.bind( 8 ) = lm_id;
                    sql.bind( 9 ) = inchi;
                    sql.bind( 10 ) = adfs::blob( ctabz.size(), ctabz.data() ) ;
                    sql.bind( 11 ) = adfs::blob( svgz.size(), svgz.data() );

                    if ( sql.step() != adfs::sqlite_done ) {
                        ADDEBUG() << "sql error: " << sql.errmsg() << "\terror count: " << ++error_c.first;
                        add_duplicated( *sqlite, mol.get(), inchiKey, inchi, smiles, formula, mass, logP, sdmol );
                    }
                    sql.reset();
                }
            } else {
                ADDEBUG() << "SDFile has no CTable: " << std::format( "[{}]\t", i )
                          << boost::json::value_from( sdmol.dataItems() )
                          << "\nerror count: " << ++error_c.second;
            }
            (*progress)(i);
            qApp->processEvents();
        } // for
        ADDEBUG() << "############ Total error counts: " << error_c;
    } else { //
        ADDEBUG() << "sql error: " << sql.errmsg();
    }
}

void
SDFileImport::impl::add_duplicated( adfs::sqlite& db
                                    , const RDKit::RWMol * mol
                                    , const std::string& inchiKey
                                    , const std::string& inchi
                                    , const std::string& smiles
                                    , const std::string& formula
                                    , double mass
                                    , double logP
                                    , const adchem::SDMol& sdmol ) const
{
    adfs::stmt sql( db );
    if ( sql.prepare("INSERT INTO duplicated_mols ("
                     " id"                // 1
                     ",inchiKey"          // 2
                     ",smiles"            // 3
                     ",formula"           // 4
                     ",mass"              // 5
                     ",SlogP"             // 6
                     ",lm_id"             // 8
                     ",itemData"          // 7
                     ",inchi"             // 9
                     ",lm_ctab"           // 10
                     ",svg"               // 11
                     ") VALUES (?,?,?,?,?,?,?,?,?,?,?)") ) {
        auto svgz =  bzip2_compress()( sdmol.svg() );
        auto ctabz = bzip2_compress()( sdmol.ctable() );

        sql.bind( 1 ) = sdmol.index();
        sql.bind( 2 ) = inchiKey;
        sql.bind( 3 ) = smiles;
        sql.bind( 4 ) = formula;
        sql.bind( 5 ) = mass;
        sql.bind( 6 ) = logP;
        sql.bind( 7 ) = std::string( itemSelector().to_value( sdmol.dataItems() ) );
        sql.bind( 8 ) = itemSelector()( sdmol.dataItems(), "LM_ID" );
        sql.bind( 9 ) = inchi;
        sql.bind( 10 ) = adfs::blob( ctabz.size(), ctabz.data() );
        sql.bind( 11 ) = adfs::blob( svgz.size(), svgz.data() );

        if ( sql.step() != adfs::sqlite_done ) {
            ADDEBUG() << "sql error: " << sql.errmsg();
        }
    }
}



/*
ABBREVIATION
 Abbreviation
 CATEGORY
 CLASS_LEVEL4
 Compound name
 EXACT_MASS
 Exact Mass
 ExactMW
 FORMULA=Formula
 LM_ID
 MAIN_CLASS
 MRM transition
 NAME
 Precursor-ion
 Product-ion
 SMILES
 SUB_CLASS
 SWISSLIPIDS_ID
 SYNONYMS
 SYSTEMATIC_NAME
 SlogP
 The number of lipid species
 各脂質分子の量 (pmol/mg-protein)
 */
