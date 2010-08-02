// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ORBMANAGER_H
#define ORBMANAGER_H

#include <tao/ORB.h>

#include <ace/Recursive_Thread_Mutex.h>
template<class T, class M> class ACE_Singleton;

class TAO_ORB_Manager;

namespace CosNaming {
	class Name;
}

namespace acewrapper {

	class ORBManager {
		~ORBManager();
		ORBManager();
	public:
		int init( int argc, char * argv[] );
		CORBA::ORB_ptr orb();
		CORBA::Object_ptr getObject( const CosNaming::Name& );
	private:
		TAO_ORB_Manager * orb_;
		ACE_Recursive_Thread_Mutex mutex_;
		friend ACE_Singleton< ORBManager, ACE_Recursive_Thread_Mutex >;
	};

}

namespace singleton {
	typedef ACE_Singleton< acewrapper::ORBManager, ACE_Recursive_Thread_Mutex > orbManager;
}



#endif // ORBMANAGER_H
