//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "sessionmanager.h"
#include "dataprocessor.h"
#include <boost/smart_ptr.hpp>

using namespace dataproc;

SessionManager * SessionManager::instance_ = 0;

SessionManager::SessionManager(QObject *parent) :
    QObject(parent)
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