// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ORBSERVANT_H
#define ORBSERVANT_H

#pragma warning ( disable: 4996 )
#include <tao/ORB.h>
#include <tao/PortableServer/PortableServer.h>
#pragma warning ( default: 4996 )
#include <string>

class TAO_ORB_Manager;

namespace acewrapper {

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

	//////////////////////////

	template<class T> class ORBServant {
	public:
		~ORBServant() { 
			if ( pMgr_)
				pMgr_->fini();
			delete pMgr_;
		}
		ORBServant( ORBServantManager * pMgr = 0 ) : pMgr_(pMgr) { }

		ORBServantManager* getServantManager() { return pMgr_; }
		void setServantManager( ORBServantManager * p ) { pMgr_ = p; }

		inline void activate() { 
            id_ = pMgr_->activate( &impl_ );
        }
		void deactivate() { 
			if ( pMgr_ ) 
				pMgr_->deactivate( id_ );
		}
		inline operator T* () { return &impl_; }
		inline operator typename T::_stub_ptr_type () { return impl_._this(); }
		inline const std::string& ior() const { return id_; }
		inline CORBA::ORB_ptr orb() { return pMgr_->orb(); }
        inline void broker_manager_ior( const std::string& ior ) { ior_broker_manager_ = ior; }
        inline const char * broker_manager_ior() const { return ior_broker_manager_.c_str(); }

	private:
		ORBServantManager * pMgr_;
		std::string id_;
        std::string ior_broker_manager_;
		T impl_;
	};

	namespace singleton {
		typedef ACE_Singleton< acewrapper::ORBServantManager, ACE_Recursive_Thread_Mutex > orbServantManager;
	}

}


#endif // ORBSERVANT_H
