/**************************************************************************
** Copyright (C) 2016-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "document.hpp"
#include "app_version.h"
#include "mainwindow.hpp"
#include "moltablewnd.hpp"
#include <adchem/sdfile.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <qtwrapper/settings.hpp>
#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QStandardItemModel>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/inchi.h>
#include <boost/filesystem.hpp>

namespace {
    struct impl {
        QSettings settings_;
        QSqlDatabase db_;
        std::shared_ptr< QStandardItemModel > model_;
        std::shared_ptr< adchem::SDFile > sdfile_;
        std::vector< adchem::SDFileData > sddata_;

        static impl& instance() {
            static impl impl_;
            return impl_;
        };

        impl() : settings_(
            QSettings::IniFormat, QSettings::UserScope
            , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR ) // "QtPlatz"
            , QLatin1String( "sdfview" ) )   {
        }
    };

}

using namespace sdfview;

document::~document()
{
}

document::document()
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
    auto path = boost::filesystem::path( impl::instance().settings_.fileName().toStdString() );
    auto dir = path.remove_filename() / "chemistry";

    boost::filesystem::path fpath = qtwrapper::settings( impl::instance().settings_ ).recentFile( "ChemistryDB", "Files" ).toStdWString();
    if ( fpath.empty() ) {
        fpath = dir / "Chemistry.db";
    }

    if ( !boost::filesystem::exists( fpath ) ) {
        ADERROR() << "dbfile: " << fpath << "\tnot found";
        return;
    }
    ADDEBUG() << "dbfile: " << fpath << "\texists";

    if ( fpath.extension() == "adfs" || fpath.extension() == "db" ) {
        auto db = std::make_shared< adfs::sqlite >();
        db->open( fpath.string().c_str(), adfs::readonly );

        adfs::stmt sql(*db);
        if ( sql.prepare( "SELECT name FROM sqlite_master WHERE type='table' AND name='mols'" ) ) {
            if ( sql.step() != adfs::sqlite_row ) {
                ADDEBUG() << "empty database";
            }
        }
    }

    QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE", "ChemistryDB" );
    db.setDatabaseName( QString::fromStdString( fpath.string() ) );

    if ( db.open() ) {
        impl::instance().db_ = std::move( db );
        emit onConnectionChanged();
    } else {
        QMessageBox::critical(0, QObject::tr("Cannot open database")
                              , QObject::tr("Unable to establish a database connection.\nClick Cancel to exit.")
                              , QMessageBox::Cancel );
    }
}

void
document::finalClose()
{
}

QSettings *
document::settings()
{
    return &impl::instance().settings_;
}

QSqlDatabase
document::sqlDatabase()
{
    return impl::instance().db_;
}

bool
document::load( const QString& file )
{
    impl::instance().sddata_.clear();

    auto sdfile = std::make_shared< adchem::SDFile >( file.toStdString() );
    if ( sdfile && *sdfile ) {
        impl::instance().sdfile_ = std::move( sdfile );
        emit onSDFileChanged();
        return true;
    }
    return false;
}

std::shared_ptr< adchem::SDFile >
document::sdfile()
{
    return impl::instance().sdfile_;
}


void
document::setSDData( std::vector< adchem::SDFileData >&& t )
{
    impl::instance().sddata_ = std::move( t );
}

const std::vector< adchem::SDFileData >&
document::sddata() const
{
    return impl::instance().sddata_;
}
