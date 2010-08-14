// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adplugin_global.h"

#pragma warning (disable: 4996)
# include <tao/ORB.h>
# include <ace/Recursive_Thread_Mutex.h>
#pragma warning (default: 4996)

template<class T, class M> class ACE_Singleton;

class TAO_ORB_Manager;

namespace CosNaming {
	class Name;
}

namespace adplugin {

	class ADPLUGINSHARED_EXPORT ORBManager {
		~ORBManager();
		ORBManager();
	public:
		int init( int argc, char * argv[] );
		CORBA::ORB_ptr orb();
		CORBA::Object_ptr getObject( const CosNaming::Name& );

        static ORBManager * instance();

	private:
		TAO_ORB_Manager * orb_;
		ACE_Recursive_Thread_Mutex mutex_;
        friend ACE_Singleton< ORBManager, ACE_Recursive_Thread_Mutex >;
	};

}

