//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "orbmanager.h"
#include <assert.h>

#pragma warning (disable: 4669)
# include <tao/Utils/ORB_Manager.h>
#pragma warning (default: 4669)

#include <acewrapper/mutex.hpp>
#include <adportable/debug.h>

using namespace adplugin;

ORBManager::~ORBManager()
{
	acewrapper::scoped_mutex_t<> lock( mutex_ );
	//delete orb_;
}

ORBManager::ORBManager() : orb_(0)
{
}

void
ORBManager::initialize( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa )
{
	orb_ = CORBA::ORB::_duplicate( orb );
    poa_ = PortableServer::POA::_duplicate( poa );
}

CORBA::ORB_ptr
ORBManager::orb()
{
	return CORBA::ORB::_duplicate( orb_.in() );
}

PortableServer::POA_ptr
ORBManager::poa()
{
    return PortableServer::POA::_duplicate( poa_.in() );
}

bool
ORBManager::deactivate( CORBA::Object_ptr obj )
{
    try {
        PortableServer::ObjectId_var object_id = poa_->reference_to_id( obj );
        poa_->deactivate_object( object_id );
    } catch ( CORBA::Exception& ex ) {
        adportable::debug( __FILE__, __LINE__ ) << ex._info().c_str();
        return false;
    }
    return true;
}

CORBA::Object_ptr
ORBManager::string_to_object( const std::string& ior )
{
	if ( CORBA::is_nil( orb_ ) )
	  return 0;
	return orb_->string_to_object( ior.c_str() );
}

adplugin::ORBManager * 
ORBManager::instance()
{
    typedef ACE_Singleton< adplugin::ORBManager, ACE_Recursive_Thread_Mutex > impl;
    return impl::instance();
}
