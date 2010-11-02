// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma warning ( disable: 4996 )
#include <tao/ORB.h>
#include <tao/PortableServer/PortableServer.h>
#pragma warning ( default: 4996 )
#include <string>

#pragma once

class TAO_ORB_Manager;

namespace servant {

	class ORBServantManager {
        ORBServantManager( const ORBServantManager& ); // noncopyable
	public:
        ~ORBServantManager();
		ORBServantManager( CORBA::ORB_ptr orb = 0
			          , PortableServer::POA_ptr = 0
					  , PortableServer::POAManager_ptr = 0);

		int init( int ac, ACE_TCHAR * av[] );
		int fini();
		void run();

		CORBA::ORB_ptr orb();
		PortableServer::POA_ptr root_poa();
		PortableServer::POA_ptr child_poa();
		PortableServer::POAManager_ptr poa_manager();

		std::string activate( PortableServer::Servant );
		void deactivate( const std::string& id );

        bool spawn();

		bool test_and_set_thread_flag();
		static void * thread_entry( void * me );

	protected:
        size_t init_count_;
		bool thread_running_;
        ACE_Recursive_Thread_Mutex mutex_;
		TAO_ORB_Manager * orbmgr_;
	};

	namespace singleton {
		typedef ACE_Singleton< ORBServantManager, ACE_Recursive_Thread_Mutex > orbServantManager;
	}

}
