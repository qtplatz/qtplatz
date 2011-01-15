// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "sessionmanager.h"
#include "dataprocessor.h"
#include <boost/smart_ptr.hpp>

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
SessionManager::addDataprocessor( boost::shared_ptr<Dataprocessor>& proc )
{
    sessions_.push_back( Session( proc ) );
    emit signalSessionAdded( proc.get() );
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

void
SessionManager::selectionChanged( Dataprocessor* dataprocessor, portfolio::Folium& folium )
{
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

Session::Session( boost::shared_ptr<Dataprocessor>& p ) : processor_( p )
{
}

Dataprocessor&
Session::getDataprocessor()
{
    return *processor_;
}

