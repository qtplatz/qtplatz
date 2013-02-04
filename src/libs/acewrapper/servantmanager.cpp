/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "servantmanager.hpp"
#include "mutex.hpp"
#include <adportable/debug.hpp>

using namespace acewrapper;

#  include <tao/Utils/ORB_Manager.h>
#  include <ace/Thread_Manager.h>

// static
ServantManager *
ServantManager::instance()
{
    return singleton::ServantManager::instance();
}

ServantManager::~ServantManager()
{
    delete orbmgr_;
    adportable::debug(__FILE__, __LINE__) << "***** ServantManager dtor complete";    
}

ServantManager::ServantManager( CORBA::ORB_ptr orb
                        , PortableServer::POA_ptr poa
                        , PortableServer::POAManager_ptr poamanager ) : init_count_(0)  
                                                                      , thread_running_(false)
                                                                      , orbmgr_(0)
                                                                      , threadid_(0)
{
    orbmgr_ = new TAO_ORB_Manager( orb, poa, poamanager );
}

int
ServantManager::init( int ac, ACE_TCHAR * av[] )
{
    acewrapper::scoped_mutex_t<> lock( mutex_ );

    if ( init_count_++ == 0 )
        return orbmgr_->init( ac, av );

    return 0;
}

int
ServantManager::fini()
{
    acewrapper::scoped_mutex_t<> lock( mutex_ );

    if ( init_count_ && --init_count_ == 0 )
        return orbmgr_->fini();

    return 0;
}

void
ServantManager::shutdown()
{
    orbmgr_->fini();
    ACE_Thread::join( threadid_, 0, 0 );
}

CORBA::ORB_ptr
ServantManager::orb()
{
    return orbmgr_->orb();
}

PortableServer::POA_ptr
ServantManager::root_poa()
{
    return orbmgr_->root_poa();
}

PortableServer::POA_ptr
ServantManager::child_poa()
{
    return orbmgr_->child_poa();
}

PortableServer::POAManager_ptr
ServantManager::poa_manager()
{
    return orbmgr_->poa_manager();
}

std::string
ServantManager::activate( PortableServer::Servant servant )
{
    CORBA::String_var id = orbmgr_->activate( servant );
    return std::string ( id.in() );
}

void
ServantManager::deactivate( const std::string& id )
{
    orbmgr_->deactivate( id.c_str() );
}

void
ServantManager::run()
{
    threadid_ = ACE_Thread::self();
    try {
	adportable::debug(__FILE__, __LINE__) << "-----> ServantManager thread started.";
        orbmgr_->run();
    } catch ( ... ) {
        adportable::debug(__FILE__, __LINE__) << "-----> ServantManager got an exception (...).";
        thread_running_ = false;
        throw;
    }
    adportable::debug(__FILE__, __LINE__) << "-----> ServantManager thread terminated.";
}

// static
void *
ServantManager::thread_entry( void * me )
{
    ServantManager * pThis = reinterpret_cast< ServantManager * >( me );
    if ( pThis && pThis->orbmgr_ )
        pThis->run();
    return 0;
}

bool
ServantManager::spawn()
{
    if ( ! thread_running_ ) {
	acewrapper::scoped_mutex_t<> lock( mutex_ );
	if ( ! thread_running_ ) {
	    thread_running_ = true;
	    ACE_Thread_Manager::instance()->spawn( ACE_THR_FUNC(ServantManager::thread_entry), this );
	    return true;
	}
    }
    return false; // already running
}

