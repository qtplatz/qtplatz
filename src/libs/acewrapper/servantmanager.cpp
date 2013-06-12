/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "servantmanager.hpp"
#include "mutex.hpp"
#include <adportable/debug.hpp>
#include <functional>

using namespace acewrapper;

#  include <tao/Utils/ORB_Manager.h>
#  include <ace/Thread_Manager.h>

ServantManager * ServantManager::instance_ = 0;
std::mutex ServantManager::mutex_;

// static
ServantManager *
ServantManager::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new ServantManager();
    }
    return instance_;
    // return singleton::ServantManager::instance();
}

ServantManager::~ServantManager()
{
    delete orbmgr_;
	delete thread_;
    adportable::debug(__FILE__, __LINE__) << "***** ServantManager dtor complete";    
}

ServantManager::ServantManager( CORBA::ORB_ptr orb
                                , PortableServer::POA_ptr poa
                                , PortableServer::POAManager_ptr poamanager ) : init_count_(0)  
                                                                              , thread_running_(false)
                                                                              , orbmgr_(0)
                                                                              , thread_(0)
{
    orbmgr_ = new TAO_ORB_Manager( orb, poa, poamanager );
}

int
ServantManager::init( int ac, ACE_TCHAR * av[] )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( init_count_++ == 0 )
        return orbmgr_->init( ac, av );

    return 0;
}

int
ServantManager::fini()
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( init_count_ && --init_count_ == 0 )
        return orbmgr_->fini();

    return 0;
}

void
ServantManager::shutdown()
{
	std::lock_guard< std::mutex > lock( mutex_ );
	if ( orbmgr_ )
		orbmgr_->fini();
	if ( thread_ )
		thread_->join();
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

bool
ServantManager::spawn()
{
    if ( ! thread_ ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( ! thread_ ) {
            thread_ = new std::thread( std::bind( &ServantManager::run, this ) );
            return true;
        }
    }
    return false; // already running
}

