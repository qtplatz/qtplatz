// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adinterface/brokerS.h"
#include <acewrapper/orbservant.h>
#include <map>
#include <string>
#include "session_i.h"
#include <boost/smart_ptr.hpp>

namespace ns_adcontroller {
	class manager_i;
}

namespace singleton {
	namespace adcontroller {
		typedef ACE_Singleton< acewrapper::ORBServant< ns_adcontroller::manager_i >, ACE_Recursive_Thread_Mutex > manager;
	}
}

namespace ns_adcontroller {

	class manager_i : public virtual POA_ControlServer::Manager {

	public:
		manager_i(void);
		~manager_i(void);

		void shutdown();
		ControlServer::Session_ptr getSession( const CORBA::WChar * );
	private:
		typedef std::map< std::wstring, boost::shared_ptr< ns_adcontroller::session_i > > session_map_type;
		session_map_type session_list_;
	};
}
