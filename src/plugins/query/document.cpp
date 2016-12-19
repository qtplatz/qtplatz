/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "queryconnection.hpp"
#include "queryconstants.hpp"
#include <adcontrols/processmethod.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adfs/filesystem.hpp>
#include <adlog/logger.hpp>
#include <adportable/profile.hpp>
#include <adpublisher/document.hpp>
#include <qtwrapper/settings.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <coreplugin/progressmanager/progressmanager.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <boost/lexical_cast.hpp>
#include <app/app_version.h>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QFuture>
#include <algorithm>

namespace query {
    namespace detail {
        struct user_preference {
            static boost::filesystem::path path( QSettings * settings ) {
                boost::filesystem::path dir( settings->fileName().toStdWString() );
                return dir.remove_filename() / "Query";                
            }
        };
    }
}

using namespace query;

std::unique_ptr< document > document::instance_;

document::~document()
{
}

document::document()
    : settings_( std::make_unique< QSettings >(QSettings::IniFormat, QSettings::UserScope
                                               , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                               , QLatin1String( "Query" ) ) )
{
    //connect( this, &document::onProcessed, this, &document::handle_processed );
}

document *
document::instance()
{
    static std::once_flag flag;
    std::call_once( flag, [] () { instance_.reset( new document() ); } );
    return instance_.get();
}

void
document::onInitialUpdate()
{
}

void
document::onFinalClose()
{
}


void
document::setConnection( QueryConnection * conn )
{
    queryConnection_ = conn->shared_from_this();

    qtwrapper::settings( *settings_ ).addRecentFiles( Constants::GRP_DATA_FILES, Constants::KEY_FILES, QString::fromStdWString( conn->filepath() ) );

    emit onConnectionChanged();
}

QueryConnection *
document::connection()
{
    return queryConnection_.get();
}

QString
document::lastDataDir() const
{
    return qtwrapper::settings( *settings_ ).recentFile( Constants::GRP_DATA_FILES, Constants::KEY_FILES );
}

void
document::addSqlHistory( const QString& sql )
{
    auto list = sqlHistory();
    
    list.erase( std::remove_if( list.begin(), list.end(), [sql] ( const QString& a ){ return sql == a; } ), list.end() );
    
    settings_->beginGroup( "Query" );
    settings_->beginWriteArray( "History" );

    settings_->setArrayIndex( 0 );
    settings_->setValue( "Sql", sql );

    for ( size_t i = 0; i < list.size() && i < 256; ++i ) {
        settings_->setArrayIndex( i + 1 );
        settings_->setValue( "Sql", list[ i ] );
    }
    
    settings_->endArray();
    settings_->endGroup();

    emit onHistoryChanged();
}


QStringList
document::sqlHistory()
{
    QStringList list;

    settings_->beginGroup( "Query" );

    auto size = settings_->beginReadArray( "History" );

    for ( int i = 0; i < size; ++i ) {
        settings_->setArrayIndex( i );
        list << settings_->value( "Sql" ).toString();
    }

    settings_->endArray();
    settings_->endGroup();
    
    return list;
}

void
document::setMassSpectrometer( std::shared_ptr< adcontrols::MassSpectrometer > sp )
{
    massSpectrometer_ = sp;
}

std::shared_ptr< adcontrols::MassSpectrometer >
document::massSpectrometer()
{
    return massSpectrometer_;
}
