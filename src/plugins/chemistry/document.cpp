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
#include "chemspider.hpp"
#include <adchem/drawing.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adprot/aminoacid.hpp>
#include <adportable/debug.hpp>
#include <app/app_version.h>
#include <qtwrapper/settings.hpp>
#include <QTextEdit>
#include <QSqlError>
#include <QSqlQuery>

#if defined _MSC_VER
# pragma warning(disable:4267) // size_t to unsigned int possible loss of data (x64 int on MSC is 32bit)
#endif

#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <GraphMol/inchi.h>
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


    static struct { std::string smiles; std::vector< std::string > synonym; } inidb[] = {
        { "C(C(C(F)(F)F)(F)F)(C(N(C(C(C(C(F)(F)F)(F)F)(F)F)(F)F)C(C(C(C(F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F", { "PFTBA" } }
        , { "[Cl-].[S+]1C2C=C(C=CC=2N=C2C=CC(=CC=12)N(C)C)N(C)C",                                         { "Methylene blue" } }
        , { "CN(C)C1C=CC(=CC=1)C(C1C=CC(=CC=1)N(C)C)=C1C=CC(C=C1)=[N+](C)C.[Cl-]",                        { "Methyl violet", "crystal violet" } }
        , { "CCN(CC)c1ccc2c(c1)oc-3cc(=[N+](CC)CC)ccc3c2c4ccccc4C(=O)O.[Cl-]",                            { "Rhodamine B" } }
        , { "O=C(Nc1ccc(OCC)cc1)C",                                                                       { "Phenacetin" } }
        , { "c1ccc2c(c1)c(=N)c3c([nH]2)CCCC3",                                                            { "Tacrine" } }
        , { "O=C(N)c1ccc[n+](c1)[C@@H]2O[C@@H]([C@@H](O)[C@H]2O)COP([O-])(=O)OP(=O)([O-])OC[C@H]5O[C@@H](n4cnc3c(ncnc34)N)[C@H](O)[C@@H]5O", { "NAD+" } }
        , { "O=C(N)C1CC=C[N](C=1)[C@@H]2O[C@@H]([C@@H](O)[C@H]2O)COP([O-])(=O)OP(=O)([O-])OC[C@H]5O[C@@H](n4cnc3c(ncnc34)N)[C@H](O)[C@@H]5O", { "NADH" } }
        , { "O=C(N)c1ccc[n+](c1)[C@H]2[C@H](O)[C@H](O)[C@H](O2)COP([O-])(=O)OP(=O)(O)OC[C@H]3O[C@@H](n4cnc5c4ncnc5N)[C@@H]([C@@H]3O)OP(=O)(O)O", { "NADP" } }
        , { "c1ccc2c(c1)ccc(=O)o2",                                                                       { "Coumarin" } }
        , { "CCCCCCCCCCCCCC(=O)O[C@H](CCCCCCCCCCC)CC(=O)O[C@@H]1[C@H]([C@@H](O[C@@H]([C@H]1OP(=O)(O)O)CO)OC[C@@H]2[C@H]([C@@H]([C@H]([C@H](O2)OP(=O)(O)O)NC(=O)C[C@@H](CCCCCCCCCCC)O)OC(=O)C[C@@H](CCCCCCCCCCC)O)O)NC(=O)C[C@@H](CCCCCCCCCCC)OC(=O)CCCCCCCCCCC", { "Lipid A" } }
        , { "c1cc(ccc1N)S(=O)(=O)Nc2ccc(nn2)Cl",     { "Sulfachlorpyridazine" } } // 285
        , { "COc1cc(nc(n1)OC)NS(=O)(=O)c2ccc(cc2)N", { "Sulfadimethoxine" } }     // 310
        , { "Cc1cc(nc(n1)NS(=O)(=O)c2ccc(cc2)N)C",   { "Sulfadimidine" } }        // 278
        , { "Cc1nnc(s1)NS(=O)(=O)c2ccc(cc2)N",       { "Sulfamethizole" } }       // 270
        , { "C[C@H](CCCC(C)C)[C@H]1CC[C@@H]2[C@@]1(CC[C@H]3[C@H]2CC=C4[C@@]3(CC[C@@H](C4)O)C)C", { "Cholesterol" } }
        , { "CN1C=NC2=C1C(=O)N(C(=O)N2C)C", { "Caffeine" } }
        , { "C[C@H](CCC=C(C)C)[C@H]1CC[C@@]2([C@@]1(CC[C@]34[C@H]2CC[C@@H]5[C@]3(C4)CC[C@@H](C5(C)C)OC(=O)/C=C/c6ccc(c(c6)OC)O)C)C", { "Oryzanol A" } }
        , { "C[C@H](CCC(=C)C(C)C)[C@H]1CCC2[C@@]1(CC[C@]34[C@]2(CC[C@@H]5[C@]3(C4)CC[C@@H](C5(C)C)OC(=O)/C=C/c6ccc(c(c6)OC)O)C)C", { "Oryzanol B" } }
        , { "CC(C)C(C)CCC(C)C1CCC2C1(CCC3C2CC=C4C3(CCC(C4)OC(=O)C=CC5=CC(=C(C=C5)O)OC)C)C", { "Campesteryl ferulate" } }
        , { "Cc1c(c2c(c(c1O)C)CC[C@@](O2)(C)CCC[C@H](C)CCC[C@H](C)CCCC(C)C)C", { "Tocopherol" } }
        , { "Oc1cc(O)c2C(=O)C(O)= C(Oc2c1)c3ccc(O)c(O)c3", { "quercetin" } }
        , { R"**(O=C(O)[C@]2(O)C[C@@H](O)[C@@H](O)[C@H](OC(=O)\C=C\c1ccc(O)c(O)c1)C2)**", { "Chlorogenic acid" } }
        , { "CCCCCCCC\\C=C/CCCCCCCC(O)=O",            { "Oleic acid" } }
        , { "CCCCCCCCCCCCCCCC(=O)O",                  { "Palmitic acid" } }
        , { "C1=CC=C2C=C3C=CC=CC3=CC2=C1",            { "Anthracene" } }
        , { "c1ccc2c(c1)ccc3c2cccc3",                 { "Phenanthrene" } }
        , { "c1cc2ccc3ccc4ccc5ccc6ccc1c7c2c3c4c5c67", { "Coronene" } }
        , { "c1ccc2c3ccccc3Cc2c1",                    { "Fluorene" } }
        , { "c1ccc-2c(c1)-c3cccc4c3c2ccc4",           { "Fluoranthene" } }
        , { "c1cc2cccc3ccc4cccc1c4c32",               { "Pyrene" } }
        , { "C1=CC=C2C=CC=CC2=C1",                    { "Naphthalene" } }
        , { "c1ccc2ccccc2c1",                         { "Naphthalene" } }
        , { "C/C(=N\\c1ccc(cc1)O)/O",                 { "Paracetamol" } }
        , { "CC(=O)Nc1ccc(O)cc1",                     { "Acetaminophen", "paracetamol" } }        //
        , { "C[N+](C)(C)CC([O-])=O",                  { "Betaine" } }
        , { "COP1(=NP(=NP(=N1)(OC)OC)(OC)OC)OC",      { "Hexamethoxyphosphazine" } }
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
    if ( path.empty() || path.filename() == "molecules.adfs" || path.filename() == "ChemistryDB.adfs" ) {
        path = dir / "Chemistry.db";
    }

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
                    bool deprecated(false);
                    sql.prepare( "PRAGMA TABLE_INFO(mols)" );

                    while ( sql.step() == adfs::sqlite_row ) {
                        if ( sql.get_column_value< std::string >( 1 ) == "uuid" )
                            deprecated = true;
                    }

                    if ( deprecated ) {
                        sql.exec( "DROP TABLE IF EXISTS synonyms" );
                        sql.exec( "DROP TABLE IF EXISTS mols" );
                        ChemSchema::createTables( sql );
                    }

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

#if !defined _DEBUG
    for ( const auto& rec : inidb ) {
        if ( auto mol = std::unique_ptr< RDKit::RWMol >( RDKit::SmilesToMol( rec.smiles ) ) ) {
            query->insert( *mol, rec.smiles, rec.synonym );
        }
    }

    for ( auto it = adprot::AminoAcid::begin(); it != adprot::AminoAcid::end(); ++it ) {
        if ( auto mol = std::unique_ptr< RDKit::RWMol >( RDKit::SmilesToMol( it->smiles() ) ) ) {
            std::vector< std::string > v { it->symbol() };
            query->insert( *mol, it->smiles(), v );
        }
    }
#endif
}

QSqlDatabase
document::sqlDatabase()
{
    return impl::sqlDatabase();
}

void
document::ChemSpiderSearch( const QString& sql, QTextEdit * edit )
{
    auto stmt = sql.toStdString();
    ADDEBUG() << stmt;
    ChemSpider cs( chemSpiderToken().toStdString() );

    if ( cs.AsyncSimpleSearch( stmt ) ) {

        int retry( 10 );
        std::string status;
        while( !cs.GetAsyncSearchStatus( status ) && retry-- )
            edit->append( QString::fromStdString( status ) );

        if ( retry <= 0 )
            return;

        cs.GetAsyncSearchResult();

        edit->setText( QString::fromStdString( cs.rid() + "\n" ) );

        for ( auto& csid: cs.csids() ) {

            edit->append( QString("csid=%1\n").arg( csid ) );
            std::string smiles, InChI, InChIKey;

            if ( cs.GetCompoundInfo( csid, smiles, InChI, InChIKey ) ) {

                // std::vector< std::string > synonyms;
                // cs.GetSynonyms( csid, synonyms );

                edit->append( QString("%1").arg( QString::fromStdString( smiles ) ) );
                edit->append( QString("%1").arg( QString::fromStdString( InChI ) ) );
                edit->append( QString("%1").arg( QString::fromStdString( InChIKey ) ) );

                if ( auto mol = std::unique_ptr< RDKit::RWMol >( RDKit::SmilesToMol( smiles ) ) ) {

                    std::string svg = adchem::drawing::toSVG( *mol );
                    std::string formula = RDKit::Descriptors::calcMolFormula( *mol, true, false );
                    double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( formula );

                    impl::sqlDatabase().transaction();

                    do {
                        QSqlQuery sql( impl::sqlDatabase() );

                        sql.prepare( "INSERT OR IGNORE INTO mols (InChI) VALUES (?)" );
                        sql.addBindValue( QString::fromStdString( InChI ) );

                        if ( !sql.exec() ) {
                            ADDEBUG() << "SQLite error: " << sql.lastError().text().toStdString()
                                      << ", code = " << sql.lastError().number() << " while inserting " << InChI;
                        }
                    } while( 0 );

                    do {
                        QSqlQuery sql( impl::sqlDatabase() );

                        sql.prepare(
                            "UPDATE mols SET csid = ?, formula = ?, mass = ?, svg = ?, smiles = ?, InChIKey = ? "
                            " WHERE InChI = ?" );

                        sql.addBindValue( csid );
                        sql.addBindValue( QString::fromStdString( formula ) );
                        sql.addBindValue( mass );
                        sql.addBindValue( QByteArray( svg.data(), int( svg.size() ) ) );
                        sql.addBindValue( QString::fromStdString( smiles ) ); // utf8 on db
                        sql.addBindValue( QString::fromStdString( InChIKey ) );
                        sql.addBindValue( QString::fromStdString( InChI ) );

                        if ( !sql.exec() ) {
                            ADDEBUG() << "SQLite error: " << sql.lastError().text().toStdString()
                                      << ", code = " << sql.lastError().number() << " while updating " << InChI;
                        }
                    } while( 0 );

                    impl::sqlDatabase().commit();
                }

            }
        }
        emit onConnectionChanged(); // this will re-run setQuery
    }
}

void
document::setChemSpiderToken( const QString& token )
{
    settings()->setValue( "ChemSpider/SecurityToken", token );
}

QString
document::chemSpiderToken() const
{
    return impl::settings()->value( "ChemSpider/SecurityToken" ).toString();
}

void
document::findCSIDFromInChI( const QString& InChI )
{
    ChemSpider cs( chemSpiderToken().toStdString() );

    if ( cs.AsyncSimpleSearch( InChI.toStdString() ) ) {

        int retry( 10 );
        std::string status;
        while( !cs.GetAsyncSearchStatus( status ) && retry-- )
            ;

        if ( retry <= 0 )
            return;

        cs.GetAsyncSearchResult();

        for ( auto& csid: cs.csids() ) {

            QSqlQuery sql( impl::sqlDatabase() );

            sql.prepare( "UPDATE mols SET csid = ? WHERE InChI = ?" );

            sql.addBindValue( csid );
            sql.addBindValue( InChI );

            if ( !sql.exec() ) {
                ADDEBUG() << "SQLite error: " << sql.lastError().text().toStdString()
                          << ", code = " << sql.lastError().number() << " while updating " << InChI.toStdString();
            }
        }
        emit onConnectionChanged(); // this will re-run setQuery
    }
}
