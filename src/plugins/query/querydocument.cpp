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
    //queryConnection_ = conn->shared_from_this();

    qtwrapper::ProgressHandler handler( 0, 5 );
    qtwrapper::waitCursor w;
#if 0
    if ( ( publisher_ = std::make_shared< QueryPublisher >() ) ) {
        
        Core::ProgressManager::addTask( handler.progress.future(), "Query connecting database...", Constants::QUERY_TASK_OPEN );
        
        std::thread work( [&] () { (*publisher_)(conn, handler); } );
        
        work.join();

        emit onConnectionChanged();
    }

    addRecentFiles( Constants::GRP_DATA_FILES, Constants::KEY_FILES, QString::fromStdWString( conn->filepath() ) );
#endif
}

QueryConnection *
QueryDocument::connection()
{
    //return queryConnection_.get();
    return 0;
}

void
QueryDocument::addRecentFiles( const QString& group, const QString& key, const QString& value )
{
    std::vector< QString > list;
    getRecentFiles( group, key, list );

    boost::filesystem::path path = boost::filesystem::path( value.toStdWString() ).generic_wstring();
    auto it = std::remove_if( list.begin(), list.end(), [path] ( const QString& a ){ return path == a.toStdWString(); } );
    if ( it != list.end() )
        list.erase( it, list.end() );

    settings_->beginGroup( group );

    settings_->beginWriteArray( key );
    settings_->setArrayIndex( 0 );
    settings_->setValue( "File", QString::fromStdWString( path.generic_wstring() ) );
    for ( size_t i = 0; i < list.size() && i < 7; ++i ) {
        settings_->setArrayIndex( int(i + 1) );
        settings_->setValue( "File", list[ i ] );
    }
    settings_->endArray();

    settings_->endGroup();
}

void
QueryDocument::getRecentFiles( const QString& group, const QString& key, std::vector<QString>& list ) const
{
    settings_->beginGroup( group );

    int size = settings_->beginReadArray( key );
    for ( int i = 0; i < size; ++i ) {
        settings_->setArrayIndex( i );
        list.push_back( settings_->value( "File" ).toString() );
    }
    settings_->endArray();

    settings_->endGroup();
}

QString
QueryDocument::recentFile( const QString& group, const QString& key ) const
{
    QString value;

    settings_->beginGroup( group );
    
    if ( int size = settings_->beginReadArray( key ) ) {
        (void)size;
        settings_->setArrayIndex( 0 );
        value = settings_->value( "File" ).toString();
    }
    settings_->endArray();
    
    settings_->endGroup();

    return value;
}

