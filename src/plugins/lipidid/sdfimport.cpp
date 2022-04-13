/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adchem/sdfile.hpp>
#include <adchem/sdmol.hpp>
#include <adchem/sdmolsupplier.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
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
#include <boost/filesystem.hpp>
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
        std::string operator()( const std::vector< std::pair< std::string, std::string > >& list ) const {
            const static std::vector< std::string > select = {
                "ABBREVIATION"
                //, "Abbreviation"
                , "CATEGORY"
                //, "CLASS_LEVEL4"
                //, "Compound name"
                //, "EXACT_MASS"
                //, "Exact Mass"
                //, "ExactMW"
                //, "FORMULA=Formula"
                , "LM_ID"
                , "MAIN_CLASS"
                //, "MRM transition"
                , "NAME"
                //, "Precursor-ion"
                //, "Product-ion"
                //, "SMILES"
                //, "SUB_CLASS"
                , "SWISSLIPIDS_ID"
                , "SYNONYMS"
                //, "SYSTEMATIC_NAME"
                //, "SlogP"
                //, "The number of lipid species"
                //, "各脂質分子の量 (pmol/mg-protein)"
            };
            std::vector< std::pair< std::string, std::string > > filterd;
            std::copy_if( list.begin(), list.end(), std::back_inserter( filterd )
                          , [&](const auto& a){
                              return
                                  std::find_if( select.begin(), select.end(), [&](const auto& t){ return t == a.first; }) != select.end();
                          });
            return boost::json::serialize( boost::json::value{{"dataItem", filterd}} );
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
        auto path = boost::filesystem::path( fn.toStdString() );
        boost::system::error_code ec;
        if ( boost::filesystem::exists( path, ec ) ) {
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
    boost::filesystem::path fpath( settings( *document::settings() ).recentFile( "LIPID_MAPS", "Files" ).toStdString() );
    if ( fpath.empty() ) {
        auto path = boost::filesystem::path( document::settings()->fileName().toStdString() );
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
                    ",svg              TEXT"
                    ",formula          TEXT"
                    ",mass             REAL"
                    ",SlogP            REAL"
                    ",inchiKey         TEXT"
                    ",smiles           TEXT"
                    ",itemData         TEXT" //  JSON
                    ",UNIQUE(inchiKey)"
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

    auto future = std::async( std::launch::async, [=](){ task( sdfile, sqlite, p ); } );

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

    if ( sql.prepare("INSERT INTO mols (id,svg,formula,mass,SlogP,inchiKey,smiles,itemData) VALUES (?,?,?,?,?,?,?,?)") ) {

        std::vector< std::tuple< size_t, std::string, double, std::string > > dbg;

        for ( size_t i = 0; i < sdfile->size(); ++i ) {
            auto sdmol     = sdfile->at( i );
            auto inchikey  = RDKit::MolToInchiKey( sdmol.mol() );
            auto svg       = sdmol.svg();
            auto smiles    = sdmol.smiles();
            auto formula   = sdmol.formula();
            double mass    = sdmol.mass();
            double logP(0);
            if ( auto a = SlogP()( sdmol.dataItems() ) )
                logP  = *a;
            else
                std::tie(logP, std::ignore) = sdmol.logP();
            auto json = itemSelector()( sdmol.dataItems() );

            if ( ( i % 1000 ) == 0 ) {
                ADDEBUG() << i << "/" << sdfile->size() << "\t" << std::make_tuple( i, formula, mass, inchikey );
            }

            int col(1);
            sql.bind( col++ ) = sdmol.index();
            sql.bind( col++ ) = svg;
            sql.bind( col++ ) = formula;
            sql.bind( col++ ) = mass;
            sql.bind( col++ ) = logP;
            sql.bind( col++ ) = inchikey;
            sql.bind( col++ ) = smiles;
            sql.bind( col++ ) = json;

            if ( sql.step() != adfs::sqlite_done ) {
                dbg.emplace_back( i, formula, mass, inchikey );
                ADDEBUG() << "sql error: " << sql.errmsg()
                          << "\t" << dbg.back()
                          << "\terror count: " << dbg.size();
            }
            sql.reset();
            (*progress)(i);
            qApp->processEvents();
        }
    } else {
        ADDEBUG() << "sql error: " << sql.errmsg();
    }

    // -- for initial schema code preparation ->
    // for ( const auto& key: dataKeys )
    //     ADDEBUG() << key;
    // <- for initial schema code preparation --
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
