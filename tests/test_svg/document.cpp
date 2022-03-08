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
#include <pugixml.hpp>
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
#include <sstream>

namespace {
    struct impl {
        QSettings settings_;
        std::shared_ptr< adfs::sqlite > sqlite_;
        std::string inchiKey_;

        QString filename_;
        pugi::xml_document xdoc_;

        std::shared_ptr< adfs::sqlite > sqlite() { return sqlite_; }

        static impl& instance() {
            static impl impl_;
            return impl_;
        };

        impl() : settings_( QSettings::IniFormat, QSettings::UserScope
                            , QLatin1String( "QtPlatz" ) // "QtPlatz"
                            , QLatin1String( "test_svg" ) )
               , sqlite_( std::make_shared< adfs::sqlite >() )
               , inchiKey_( "YANLIXDCINUGHT-UHFFFAOYSA-N" ) {
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

    using qtwrapper::settings;
    std::filesystem::path fpath
        = settings( impl::instance().settings_ ).recentFile( "ChemistryDB", "Files" ).toStdWString();
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
        // ADDEBUG() << sql.get_column_value< std::string >( 0 );
    }

    //sql.prepare( "SELECT svg FROM mols WHERE inchiKey = 'YANLIXDCINUGHT-UHFFFAOYSA-N'" );
    sql.prepare( "SELECT svg FROM mols WHERE inchiKey = ?");
    sql.bind( 1 ) = impl::instance().inchiKey_;
    if ( sql.step() == adfs::sqlite_row ) {
        auto [ svg ] = adfs::get_column_values< std::string >( sql );
        setSvg( svg );
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

std::string
document::svg() const
{
    std::ostringstream o;
    impl::instance().xdoc_.save( o );
    return o.str();
}

void
document::setSvg( const std::string& svg )
{
    if ( impl::instance().xdoc_.load_string( svg.c_str() ) )
        ADDEBUG() << "svg parse ok";
    else
        ADDEBUG() << "svg parse failed";
}

void
document::saveSvg( const QString& filename ) const
{
    if ( impl::instance().xdoc_.save_file( filename.toStdString().c_str() ) )
        impl::instance().filename_ = filename;
}

bool
document::loadSvg( const QString& filename )
{
    auto path = std::filesystem::path( filename.toStdString() );
    if ( std::filesystem::exists( path ) ) {
        pugi::xml_document doc;
        if ( doc.load_file( path.c_str() ) ) {
            impl::instance().xdoc_ = std::move( doc );
            impl::instance().filename_ = filename;

            std::ostringstream o;
            impl::instance().xdoc_.save( o );
            emit onSvgLoaded( QByteArray( o.str().data(), o.str().size() ) );
            return true;
        }
    }
    return false;
}

QString
document::filename() const
{
    return impl::instance().filename_;
}

void
document::setInChIKey( const std::string& key )
{
    impl::instance().inchiKey_ = key;
}

void
document::editSvg()
{
    auto& doc = impl::instance().xdoc_;
    ADDEBUG() << "----------- edit svg ---------------" << doc;
    if ( doc ) {
        auto svg = doc.select_node( "/svg" );
        auto node = svg.node().append_child();
        node.set_name( "text" );
        node.set_value( "value" );
        node.append_attribute( "x" ).set_value( 200 );
        node.append_attribute( "y" ).set_value( 50 );
        node.append_attribute( "style" ).set_value( "font-size:13px;font-style:normal" );
        //node.append_child( pugi::node_pcdata ).set_value( R"(H<tspan baseline-shift='sub'>3</tspan>O<tspan baseline-shift='super'>+</tspan>)" );
        node.append_child( pugi::node_pcdata ).set_value( "H" );
        auto ts = node.append_child( "tspan" );
        ts.append_attribute( "dy" ).set_value( -7 );
        ts.append_child( pugi::node_pcdata ).set_value( "3" );

        std::ostringstream o;
        doc.print( o, "" ); //, pugi::format_raw);
        ADDEBUG() << o.str();
        emit onSvgLoaded( QByteArray( o.str().data(), o.str().size() ) );
    }
	// <text x="257.8" y="282.7" class="atom-10"
    // style="font-size:13px;font-style:normal;font-weight:normal;fill-opacity:1;stroke:no
}

void
document::walkSvg()
{
    auto& doc = impl::instance().xdoc_;
    if ( doc ) {
        for (auto point : doc.select_nodes("/svg/text")) {
             std::ostringstream o;
             point.node().print( o, "" ); //, pugi::format_raw);
             ADDEBUG() << o.str()
                       << "\n" << point.node().attribute( "class" ).value();
        }
    }
}
