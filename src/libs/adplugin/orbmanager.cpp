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
	return 0;
}

CORBA::ORB_ptr
ORBManager::orb()
{
   if ( orb_ )
	   return orb_->orb();
   return 0;
}

#if defined USE_NAMING_SERVICE
CORBA::Object_ptr
ORBManager::getObject( const CosNaming::Name& name )
{
   if ( ! orb_ )
	   return 0;
   CosNaming::NamingContext_var nc = acewrapper::NS::resolve_init( orb_->orb() );
   return acewrapper::NS::resolve_name( nc, name );
}

CORBA::Object_ptr
ORBManager::getObject( const std::wstring& naming )
{
   if ( ! orb_ )
	   return 0;
   return acewrapper::NS::resolve_name( orb_->orb(), naming );
}
#endif

CORBA::Object_ptr
ORBManager::string_to_object( const std::string& ior )
{
  if ( ! orb_ )
	  return 0;
  return orb_->orb()->string_to_object( ior.c_str() );
}

adplugin::ORBManager * 
ORBManager::instance()
{
    typedef ACE_Singleton< adplugin::ORBManager, ACE_Recursive_Thread_Mutex > impl;
    return impl::instance();
}
