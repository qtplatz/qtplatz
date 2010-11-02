//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "orbservantmanager.h"
#include <acewrapper/mutex.hpp>

# pragma warning (disable: 4996)
#  include <tao/Utils/ORB_Manager.h>
#  include <ace/Thread_Manager.h>
# pragma warning (default: 4996)

using namespace servant;

ORBServantManager::~ORBServantManager()
{
}

ORBServantManager::ORBServantManager( CORBA::ORB_ptr orb
					   , PortableServer::POA_ptr poa
					   , PortableServer::POAManager_ptr poamanager ) : orbmgr_(0)
					                                                 , thread_running_(false)
																	 , init_count_(0)  
{
	orbmgr_ = new TAO_ORB_Manager( orb, poa, poamanager );
}

int
ORBServantManager::init( int ac, ACE_TCHAR * av[] )
{
	acewrapper::scoped_mutex_t<> lock( mutex_ );

	if ( init_count_++ == 0 )
		return orbmgr_->init( ac, av );
	return 0;
}

int
ORBServantManager::fini()
{
	acewrapper::scoped_mutex_t<> lock( mutex_ );

	if ( init_count_ && --init_count_ == 0 )
		return orbmgr_->fini();
	return 0;
}

CORBA::ORB_ptr
ORBServantManager::orb()
{
	return orbmgr_->orb();
}

PortableServer::POA_ptr
ORBServantManager::root_poa()
{
	return orbmgr_->root_poa();
}

PortableServer::POA_ptr
ORBServantManager::child_poa()
{
	return orbmgr_->child_poa();
}

PortableServer::POAManager_ptr
ORBServantManager::poa_manager()
{
	return orbmgr_->poa_manager();
}

std::string
ORBServantManager::activate( PortableServer::Servant servant )
{
	CORBA::String_var id = orbmgr_->activate( servant );
	return std::string ( id.in() );
}

void
ORBServantManager::deactivate( const std::string& id )
{
	orbmgr_->deactivate( id.c_str() );
}

bool
ORBServantManager::test_and_set_thread_flag()
{
	acewrapper::scoped_mutex_t<> lock( mutex_ );
	if ( thread_running_ )
		return false;
	thread_running_ = true;
	return true;
}

void
ORBServantManager::run()
{
	try {
		orbmgr_->run();
		thread_running_ = false;
	} catch ( ... ) {
		thread_running_ = false;
		throw;
	}
}

// static
void *
ORBServantManager::thread_entry( void * me )
{
	ORBServantManager * pThis = reinterpret_cast< ORBServantManager * >( me );
	if ( pThis && pThis->orbmgr_ )
		pThis->run();
	return 0;
}

bool
ORBServantManager::spawn()
{
    if ( this->test_and_set_thread_flag() ) {
        ACE_Thread_Manager::instance()->spawn(
            ACE_THR_FUNC( ORBServantManager::thread_entry )
            , reinterpret_cast<void *>(this)
            );
        return true;
    }
    return false;
}

