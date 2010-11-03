// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adplugin_global.h"

#pragma warning (disable: 4996)
# include <tao/ORB.h>
# include <tao/PortableServer/PortableServer.h>
# include <ace/Recursive_Thread_Mutex.h>
#pragma warning (default: 4996)

#include <string>

template<class T, class M> class ACE_Singleton;

class TAO_ORB_Manager;

namespace adplugin {

	class ADPLUGINSHARED_EXPORT ORBManager {
		~ORBManager();
		ORBManager();

	public:
		// int init( int argc, char * argv[] );
        void initialize( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa );

		CORBA::ORB_ptr orb();
        PortableServer::POA_ptr poa();
		CORBA::Object_ptr string_to_object( const std::string& ior );
        bool deactivate( CORBA::Object_ptr );

        static ORBManager * instance();

	private:
		CORBA::ORB_var orb_;
        PortableServer::POA_var poa_;

		ACE_Recursive_Thread_Mutex mutex_;
        friend ACE_Singleton< ORBManager, ACE_Recursive_Thread_Mutex >;
	};

}

