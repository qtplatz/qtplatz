// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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
#include <qtwrapper/qstring.hpp>
#include <adcontrols/datafile.hpp>
#include <adextension/isessionmanager.hpp>
#include <adportfolio/folium.hpp>
#include <coreplugin/editormanager/editormanager.h>
#include <adlog/logger.hpp>
#include <boost/any.hpp>


using namespace dataproc;

SessionManager * SessionManager::instance_ = 0;

SessionManager::SessionManager(QObject *parent) : QObject(parent)
                                                , activeDataprocessor_(0)
                                                , loadInprogress_( false )
                                                , iSessionManager_( new adextension::iSessionManager() )
{
    instance_ = this;
}

SessionManager::~SessionManager()
{
    instance_ = 0;
}

SessionManager * SessionManager::instance()
{
    return instance_;
}

adextension::iSessionManager *
SessionManager::getISessionManager()
{
    return iSessionManager_; //.get();
}

void
SessionManager::removeEditor( Core::IEditor * editor )
{
    auto it = std::find_if( sessions_.begin(), sessions_.end(), [=]( Session& s ){
            return s.editor() == editor;
        });
    if ( it != sessions_.end() ) {
        if ( activeDataprocessor_ == it->processor() )
            activeDataprocessor_ = 0;
        emit onSessionRemoved( it->processor() );
        sessions_.erase( it );
    }
}

void
SessionManager::addDataprocessor( std::shared_ptr<Dataprocessor>& proc, Core::IEditor * editor )
{
    loadInprogress_ = true;
    sessions_.push_back( Session( proc, editor ) );
	activeDataprocessor_ = proc.get();
	emit signalAddSession( proc.get() );
    emit signalSessionAdded( proc.get() );
    emit iSessionManager_->addProcessor( proc->filename() );
    loadInprogress_ = false;
}

void
SessionManager::updateDataprocessor( Dataprocessor* dataprocessor, portfolio::Folium& folium )
{
    activeDataprocessor_ = dataprocessor;
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
    if ( ! loadInprogress_ )
        emit signalCheckStateChanged( dataprocessor, folium, isChecked );
}

SessionManager::vector_type::iterator
SessionManager::begin()
{
    return sessions_.begin();
}

SessionManager::vector_type::iterator
SessionManager::end()
{
    return sessions_.end();
}

SessionManager::vector_type::iterator
SessionManager::find( const std::wstring& token )
{
    for ( SessionManager::vector_type::iterator it = sessions_.begin(); it != sessions_.end(); ++it ) {
        Dataprocessor& proc = it->getDataprocessor();
        if ( proc.file()->filename() == token )
            return it;
    }
    return sessions_.end();
}

void
SessionManager::processed( Dataprocessor* dataprocessor, portfolio::Folium& folium )
{
    emit onProcessed( dataprocessor, folium );
}

void
SessionManager::selectionChanged( Dataprocessor* dataprocessor, portfolio::Folium& folium )
{
	if ( activeDataprocessor_ != dataprocessor ) {
        activeDataprocessor_ = dataprocessor;
		auto it = std::find_if( sessions_.begin(), sessions_.end(), [dataprocessor]( dataproc::Session& s ){
                return dataprocessor == s.processor();
            });
		if ( it != sessions_.end() )
			Core::EditorManager::instance()->activateEditor( it->editor() );
	}
    emit signalSelectionChanged( dataprocessor, folium );
}

Dataprocessor *
SessionManager::getActiveDataprocessor()
{
    return activeDataprocessor_;
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

Session::Session( std::shared_ptr<Dataprocessor>& p, Core::IEditor * editor ) : processor_( p )
	                                                                          , editor_( editor )
{
}

Dataprocessor&
Session::getDataprocessor()
{
    return *processor_;
}

