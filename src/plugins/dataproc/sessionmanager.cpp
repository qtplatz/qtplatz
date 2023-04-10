// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "sessionmanager.hpp"
#include "dataprocessor.hpp"
#include <adcontrols/datafile.hpp>
#include <adextension/isessionmanager.hpp>
#include <adportfolio/folium.hpp>
#include <coreplugin/editormanager/editormanager.h>
#include <adlog/logger.hpp>
#include <qtwrapper/utils_filepath.hpp>
#include <boost/any.hpp>
#include <QSignalBlocker>
#include <mutex>
#include <thread>
#include <adportable/debug.hpp>

namespace dataproc {

    class SessionManager::impl {
    public:
        impl() : loadInprogress_( false )
               , activeDataprocessor_( 0 ) {
        }
        std::vector< Session > sessions_;
        bool loadInprogress_;
        Dataprocessor * activeDataprocessor_;
    };
}

using namespace dataproc;

namespace {
    static SessionManager * __instance;
    static std::once_flag __flag;
}


SessionManager::SessionManager( QObject *parent ) : iSessionManager( parent )
                                                  , impl_( std::make_unique< impl >() )
{
}

SessionManager::~SessionManager()
{
    ADDEBUG() << "============== SessionManager::dtor =================";
}

SessionManager *
SessionManager::instance()
{
    std::call_once( __flag, [&](){
        __instance = new SessionManager();
    });
    return __instance;
}

void
SessionManager::removeEditor( Core::IEditor * editor )
{
    auto it = std::find_if( impl_->sessions_.begin(), impl_->sessions_.end(), [&]( const auto& s ){
        return s.processor() == editor->document();
    });

    if ( it != impl_->sessions_.end() ) {
        auto filePath = it->processor()->filePath();
        if ( impl_->activeDataprocessor_ == it->processor() ) {
            impl_->activeDataprocessor_ = 0;
            emit onDataprocessorChanged( impl_->activeDataprocessor_ );
        }
        emit onRemoveSession( it->processor() );
        impl_->sessions_.erase( it );
        emit onSessionRemoved( filePath.toString() );
    }
}

void
SessionManager::addDataprocessor( std::shared_ptr<Dataprocessor>&& proc )
{
    impl_->loadInprogress_ = true; // block check state events

    impl_->sessions_.emplace_back( Session( proc, nullptr ) );
	impl_->activeDataprocessor_ = proc.get();
    emit onDataprocessorChanged( impl_->activeDataprocessor_ );

	emit signalAddSession( proc.get() );
    emit onSessionAdded( proc.get() );

    // iSessionManager
    emit addProcessor( this, qtwrapper::filepath::toString( proc->filePath() ) );

    impl_->loadInprogress_ = false;
}

void
SessionManager::updateDataprocessor( Dataprocessor* dataprocessor, portfolio::Folium& folium )
{
    if ( impl_->activeDataprocessor_ != dataprocessor ) {
        impl_->activeDataprocessor_ = dataprocessor;
        emit onDataprocessorChanged( impl_->activeDataprocessor_ );
    }
    emit onSessionUpdated( dataprocessor, QString::fromStdWString( folium.id() ) );
}

void
SessionManager::folderChanged( Dataprocessor* dataprocessor, const std::wstring& folder )
{
	emit onFolderChanged( dataprocessor, QString::fromStdWString( folder ) );
}

void
SessionManager::checkStateChanged( Dataprocessor * dataprocessor, portfolio::Folium& folium, bool isChecked )
{
    if ( ! impl_->loadInprogress_ ) {
        emit signalCheckStateChanged( dataprocessor, folium, isChecked );
        emit onCheckStateChanged( this, dataprocessor->filePath().toString(), folium, isChecked );
    }
}

SessionManager::vector_type::iterator
SessionManager::begin()
{
    return impl_->sessions_.begin();
}

SessionManager::vector_type::iterator
SessionManager::end()
{
    return impl_->sessions_.end();
}

SessionManager::vector_type::iterator
SessionManager::find( const std::wstring& token )
{
    for ( SessionManager::vector_type::iterator it = impl_->sessions_.begin(); it != impl_->sessions_.end(); ++it ) {
        auto proc = it->processor();
        if ( proc && proc->filename() == token )
            return it;
    }
    return impl_->sessions_.end();
}

void
SessionManager::processed( Dataprocessor* dataprocessor, portfolio::Folium& folium )
{
    emit onProcessed( dataprocessor, folium );

    // iSessionManager
#if QTC_VERSION >= 0x08'00'00
    emit static_cast< iSessionManager * >(this)->onProcessed( this, dataprocessor->filePath().toString(), folium );
#else
    emit static_cast< iSessionManager * >(this)->onProcessed( this, dataprocessor->filepath(), folium );
#endif
}

void
SessionManager::selectionChanged( Dataprocessor* dataprocessor, portfolio::Folium& folium )
{
	if ( impl_->activeDataprocessor_ != dataprocessor ) {
        impl_->activeDataprocessor_ = dataprocessor;
        emit onDataprocessorChanged( impl_->activeDataprocessor_ );
		auto it = std::find_if( impl_->sessions_.begin(), impl_->sessions_.end(), [dataprocessor]( dataproc::Session& s ){
                return dataprocessor == s.processor();
            });
		if ( it != impl_->sessions_.end() )
			Core::EditorManager::instance()->activateEditor( it->editor() );
	}
    emit signalSelectionChanged( dataprocessor, folium );

    // iSessionManager
#if QTC_VERSION >= 0x08'00'00
    emit onSelectionChanged( this, dataprocessor->filePath().toString(), folium );
#else
    emit onSelectionChanged( this, dataprocessor->filepath(), folium );
#endif
}

Dataprocessor *
SessionManager::getActiveDataprocessor()
{
    return impl_->activeDataprocessor_;
}

// iSessionManager implementation
std::shared_ptr< adprocessor::dataprocessor >
SessionManager::getDataprocessor( const QString& name )
{
#if QTC_VERSION >= 0x08'00'00
    auto it = std::find_if( begin(), end(), [&]( const Session& a ){ return a.processor()->filePath().toString() == name; } );
#else
    auto it = std::find_if( begin(), end(), [&]( const Session& a ){ return a.processor()->filepath() == name; } );
#endif
    if ( it != end() )
        return it->processor()->shared_from_this();

    return nullptr;
}

//////////// Session //////////////////


Session::~Session()
{
}

Session::Session() : editor_( 0 )
{
}

Session::Session( const Session& t ) : processor_( t.processor_ )
	                                 , editor_( t.editor_ )
{
}

Session::Session( std::shared_ptr<Dataprocessor> p, Core::IEditor * editor ) : processor_( p )
	                                                                          , editor_( editor )
{
}
