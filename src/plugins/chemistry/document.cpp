/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "document.hpp"
#include "chemconnection.hpp"
#include "chemquery.hpp"
#include "chemschema.hpp"
#include <adchem/drawing.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adprot/aminoacid.hpp>
#include <app/app_version.h>
#include <qtwrapper/settings.hpp>

#if defined _MSC_VER
# pragma warning(disable:4267) // size_t to unsigned int possible loss of data (x64 int on MSC is 32bit)
#endif

#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#if defined _MSC_VER
# pragma warning(default:4267) // size_t to unsigned int possible loss of data (x64 int on MSC is 32bit)
#endif

#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <boost/filesystem.hpp>

namespace chemistry {

    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "chemistry";
        }
    };

    struct document::impl {

        impl() : settings_( std::make_unique< QSettings >(
                                QSettings::IniFormat, QSettings::UserScope
                                , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                , QLatin1String( "chemistry" ) ) ) {
        }
        
        std::unique_ptr< QSettings > settings_;
        QSqlDatabase db_;

        static QSettings * settings() { return impl::instance().settings_.get(); }
        static QSqlDatabase sqlDatabase() { return impl::instance().db_; }

        static impl& instance() {
            static impl __impl;
            return __impl;
        }
    };


    static struct { std::string smiles; std::string synonym; } inidb[] = {
        { "C(C(C(F)(F)F)(F)F)(C(N(C(C(C(C(F)(F)F)(F)F)(F)F)(F)F)C(C(C(C(F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F", "PFTBA" }
        , { "[Cl-].[S+]1C2C=C(C=CC=2N=C2C=CC(=CC=12)N(C)C)N(C)C",                                         "Methylene blue" }
        , { "CN(C)C1C=CC(=CC=1)C(C1C=CC(=CC=1)N(C)C)=C1C=CC(C=C1)=[N+](C)C.[Cl-]",                        "Methyl violet; Crystal violet" }
        , { "CCN(CC)c1ccc2c(c1)oc-3cc(=[N+](CC)CC)ccc3c2c4ccccc4C(=O)O.[Cl-]",                            "Rhodamine B" }
        , { "C/C(=N\\c1ccc(cc1)O)/O",                                                                     "Paracetamol" }
        , { "c1ccc2c(c1)c(=N)c3c([nH]2)CCCC3",                                                            "Tacrine" }
        , { "CCCCCCCCCCCCCC(=O)O[C@H](CCCCCCCCCCC)CC(=O)O[C@@H]1[C@H]([C@@H](O[C@@H]([C@H]1OP(=O)(O)O)CO)OC[C@@H]2[C@H]([C@@H]([C@H]([C@H](O2)OP(=O)(O)O)NC(=O)C[C@@H](CCCCCCCCCCC)O)OC(=O)C[C@@H](CCCCCCCCCCC)O)O)NC(=O)C[C@@H](CCCCCCCCCCC)OC(=O)CCCCCCCCCCC", "Lipid A" }
    };

}

using namespace chemistry;

std::mutex document::mutex_;

document::document(QObject *parent) : QObject(parent)
{
}

document * 
document::instance()
{
    static document __instance;
    return &__instance;
}

void
document::initialSetup()
{
    boost::filesystem::path dir = user_preference::path( impl::settings() );

    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "chemistry::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
        }
    }

    boost::filesystem::path path = qtwrapper::settings( *impl::settings() ).recentFile( "ChemistryDB", "Files" ).toStdWString();
    if ( path.empty() )
        path = dir / "molecules.adfs";
    
    if ( !boost::filesystem::exists( path ) ) {

        if ( !adfs::filesystem().create( path ) ) // create qtplatz ini-db
            return;
    }

    if ( auto connection = std::make_shared< ChemConnection >() ) {

        if ( connection->connect( path ) ) {

            adfs::stmt sql = connection->db();
            if ( sql.prepare( "SELECT name FROM sqlite_master WHERE type='table' AND name='mols'" ) ) {
                if ( sql.step() != adfs::sqlite_row ) {
                    if ( ChemSchema::createTables( sql ) )
                        dbInit( connection.get() );
                } else {
                    dbInit( connection.get() );
                }
            }

            qtwrapper::settings( *impl::settings() ).addRecentFiles( "ChemistryDB", "Files", QString::fromStdWString( path.wstring() ) );
        }
    }

    QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE", "ChemistryDB" );
    db.setDatabaseName( QString::fromStdString( path.string() ) );

    if ( db.open() ) {

        impl::instance().db_ = db;

        emit onConnectionChanged();

    } else {

        QMessageBox::critical(0, tr("Cannot open database"),
                              tr("Unable to establish a database connection.\nClick Cancel to exit.")
                              , QMessageBox::Cancel );
    } 
}

void
document::finalClose()
{
    boost::filesystem::path dir = user_preference::path( impl::settings() );
    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "chemistry::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return;
        }
    }
}

QSettings *
document::settings()
{
    return impl::settings();
}

void
document::dbUpdate( ChemConnection * connection )
{
    dbInit( connection );
}

void
document::dbInit( ChemConnection * connection )
{
    auto self( connection->shared_from_this() );

    auto query = std::make_shared< ChemQuery >( connection->db() );

    for ( const auto& rec : inidb ) {
        if ( auto mol = std::unique_ptr< RDKit::RWMol >( RDKit::SmilesToMol( rec.smiles ) ) ) {
            std::string svg = adchem::drawing::toSVG( *mol );
            std::string formula = RDKit::Descriptors::calcMolFormula( *mol, true, false );
            double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( formula );

            query->insert_mol( rec.smiles, svg, formula, mass, rec.synonym );
        }
    }

    for ( auto it = adprot::AminoAcid::begin(); it != adprot::AminoAcid::end(); ++it ) {
        if ( auto mol = std::unique_ptr< RDKit::RWMol >( RDKit::SmilesToMol( it->smiles() ) ) ) {
            std::string svg = adchem::drawing::toSVG( *mol );
            std::string formula = adcontrols::ChemicalFormula::standardFormula( std::string( it->formula(false) ) + "H2O" );
            double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( it->formula() );
            query->insert_mol( it->smiles(), svg, formula, mass, it->symbol() );
        }
    }
    
}

QSqlDatabase
document::sqlDatabase()
{
    return impl::sqlDatabase();
}
