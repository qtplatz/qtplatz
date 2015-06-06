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

#include "querydocument.hpp"
#include "queryconnection.hpp"
#include "queryconstants.hpp"
//#include "paneldata.hpp"
//#include "querydatawriter.hpp"
//#include "querysampleprocessor.hpp"
//#include "queryprocessor.hpp"
//#include "queryprogress.hpp"
#include <adcontrols/processmethod.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adfs/filesystem.hpp>
#include <adlog/logger.hpp>
#include <adportable/profile.hpp>
#include <adpublisher/document.hpp>
#include <qtwrapper/settings.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <qtwrapper/progresshandler.hpp>
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

std::atomic< QueryDocument * > QueryDocument::instance_( 0 );
std::mutex QueryDocument::mutex_;

QueryDocument::~QueryDocument()
{
}

QueryDocument::QueryDocument() : settings_( std::make_shared< QSettings >(QSettings::IniFormat, QSettings::UserScope
                                                                        , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                                        , QLatin1String( "Query" ) ) )
{
    //connect( this, &QueryDocument::onProcessed, this, &QueryDocument::handle_processed );
}

QueryDocument *
QueryDocument::instance()
{
    QueryDocument * tmp = instance_.load( std::memory_order_relaxed );
    std::atomic_thread_fence( std::memory_order_acquire );
    if ( tmp == nullptr ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        tmp = instance_.load( std::memory_order_relaxed );
        if ( tmp == nullptr ) {
            tmp = new QueryDocument();
            std::atomic_thread_fence( std::memory_order_release );
            instance_.store( tmp, std::memory_order_relaxed );
        }
    }
    return tmp;
}

void
QueryDocument::onInitialUpdate()
{
}

void
QueryDocument::onFinalClose()
{
}


void
QueryDocument::setConnection( QueryConnection * conn )
{
    queryConnection_ = conn->shared_from_this();

    //qtwrapper::ProgressHandler handler( 0, 5 );
    //qtwrapper::waitCursor w;
    //if ( ( publisher_ = std::make_shared< QueryPublisher >() ) ) {
        
    //Core::ProgressManager::addTask( handler.progress.future(), "Query connecting database...", Constants::QUERY_TASK_OPEN );
    //std::thread work( [&] () { ( *publisher_ )( conn, handler ); } );
    //work.join();
    qtwrapper::settings( *settings_ ).addRecentFiles( Constants::GRP_DATA_FILES, Constants::KEY_FILES, QString::fromStdWString( conn->filepath() ) );

    emit onConnectionChanged();
}

QueryConnection *
QueryDocument::connection()
{
    return queryConnection_.get();
}

// void
// QueryDocument::addRecentFiles( const QString& group, const QString& key, const QString& value )
// {
//     qtwrapper::settings( *settings_ ).addRecentFiles( group, key, value );
// }

// void
// QueryDocument::getRecentFiles( const QString& group, const QString& key, std::vector<QString>& list ) const
// {
//     qtwrapper::settings( *settings_ ).getRecentFiles( group, key, list );
// }

// QString
// QueryDocument::recentFile( const QString& group, const QString& key ) const
// {
//     return qtwrapper::settings( *settings_ ).recentFile( group, key );    
// }

QString
QueryDocument::lastDataDir() const
{
    return qtwrapper::settings( *settings_ ).recentFile( Constants::GRP_DATA_FILES, Constants::KEY_FILES );
}
