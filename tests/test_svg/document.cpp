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
#include "mainwindow.hpp"
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adfs/get_column_values.hpp>
#include <qtwrapper/settings.hpp>
#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QStandardItemModel>
#include <filesystem>

namespace {
    struct impl {
        QSettings settings_;
        std::shared_ptr< adfs::sqlite > sqlite_;

        std::shared_ptr< adfs::sqlite > sqlite() { return sqlite_; }

        static impl& instance() {
            static impl impl_;
            return impl_;
        };

        impl() : settings_(
            QSettings::IniFormat, QSettings::UserScope
            , QLatin1String( "QtPlatz" ) // "QtPlatz"
            , QLatin1String( "test_svg" ) )
               , sqlite_( std::make_shared< adfs::sqlite >() ) {
        }
    };

}

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
    auto path = std::filesystem::path( impl::instance().settings_.fileName().toStdString() );
    auto dir = path.remove_filename() / "lipidid";

    std::filesystem::path fpath = qtwrapper::settings( impl::instance().settings_ ).recentFile( "ChemistryDB", "Files" ).toStdWString();
    if ( fpath.empty() ) {
        fpath = dir / "lipids.db";
    }

    if ( !std::filesystem::exists( fpath ) ) {
        ADERROR() << "dbfile: " << fpath << "\tnot found";
        return;
    }
    ADDEBUG() << "dbfile: " << fpath << "\texists";

    auto db = impl::instance().sqlite(); // std::make_shared< adfs::sqlite >();
    db->open( fpath.string().c_str(), adfs::readonly );
    adfs::stmt sql(*db);
    if ( sql.prepare( "SELECT name FROM sqlite_master WHERE type='table' AND name='mols'" ) ) {
        if ( sql.step() != adfs::sqlite_row ) {
            ADDEBUG() << "empty database";
        }
        ADDEBUG() << sql.get_column_value< std::string >( 0 );
    }

    sql.prepare( "SELECT svg FROM mols WHERE inchiKey = 'YANLIXDCINUGHT-UHFFFAOYSA-N'" );
    if ( sql.step() == adfs::sqlite_row ) {
        auto [ svg ] = adfs::get_column_values< std::string >( sql );
        emit onSvgLoaded( QByteArray( svg.data(), svg.size() ) );
    } else {
        ADDEBUG() << "error: " << sql.errmsg() << "\n" << sql.expanded_sql();
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
