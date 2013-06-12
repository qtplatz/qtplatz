// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include <adportable/debug.hpp>
#include <boost/any.hpp>
#include <portfolio/folium.hpp>

using namespace dataproc;

SessionManager * SessionManager::instance_ = 0;

SessionManager::SessionManager(QObject *parent) : QObject(parent)
                                                , activeDataprocessor_(0)
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

void
SessionManager::addDataprocessor( std::shared_ptr<Dataprocessor>& proc )
{
    sessions_.push_back( Session( proc ) );
	activeDataprocessor_ = proc.get();
    emit signalSessionAdded( proc.get() );
}

void
SessionManager::updateDataprocessor( Dataprocessor* dataprocessor, portfolio::Folium& folium )
{
    activeDataprocessor_ = dataprocessor;
	emit signalSessionUpdated( dataprocessor, folium );
	// emit signalSelectionChanged( dataprocessor, folium );
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
        if ( proc.file().filename() == token )
            return it;
    }
    return sessions_.end();
}

void
SessionManager::selectionChanged( Dataprocessor* dataprocessor, portfolio::Folium& folium )
{
    adportable::debug(__FILE__, __LINE__) << static_cast<boost::any&>(folium).type().name() << ", id=" << folium.id();
    activeDataprocessor_ = dataprocessor;
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

Session::Session()
{
}

Session::Session( const Session& t ) : processor_( t.processor_ )
{
}

Session::Session( std::shared_ptr<Dataprocessor>& p ) : processor_( p )
{
}

Dataprocessor&
Session::getDataprocessor()
{
    return *processor_;
}

