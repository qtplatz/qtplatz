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

using namespace adplugin;

ORBManager::~ORBManager()
{
	acewrapper::scoped_mutex_t<> lock( mutex_ );
	//delete orb_;
}

ORBManager::ORBManager() : orb_(0)
{
}

/*
int
ORBManager::init( int argc, char * argv[] )
{
	if ( orb_ == 0 ) {
		acewrapper::scoped_mutex_t<> lock( mutex_ );
		if ( orb_ == 0 ) {
			orb_ = new TAO_ORB_Manager();
			try {
				if ( orb_ ) {
					orb_->init(argc, argv);
					return argc;
				}
			} catch ( CORBA::Exception& ) {
				return (-1);
			}
		}
	}
	return 0;
}
*/

void
ORBManager::initialize( CORBA::ORB_ptr orb )
{
	orb_ = CORBA::ORB::_duplicate( orb );
}

CORBA::ORB_ptr
ORBManager::orb()
{
	return CORBA::ORB::_duplicate( orb_.in() );
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
