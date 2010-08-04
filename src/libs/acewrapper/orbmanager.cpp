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
#include <acewrapper/nameservice.h>

using namespace acewrapper;

ORBManager::~ORBManager()
{
	acewrapper::scoped_mutex_t<> lock( mutex_ );
	delete orb_;
}

ORBManager::ORBManager() : orb_(0)
{
}

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
	return (-1);
}

CORBA::ORB_ptr
ORBManager::orb()
{
   if ( orb_ )
	   return orb_->orb();
   return 0;
}

CORBA::Object_ptr
ORBManager::getObject( const CosNaming::Name& name )
{
   if ( ! orb_ )
	   return 0;
   CosNaming::NamingContext_var nc = NS::resolve_init( orb_->orb() );
   return NS::resolve_name( nc, name );
}
