// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#pragma warning (disable: 4996)
#include "adinterface/brokerS.h"
#pragma warning (default: 4996)

#include "session_i.h"
#include <acewrapper/orbservant.h>
#include <map>
#include <string>
#include <boost/smart_ptr.hpp>

namespace adcontroller {

	class manager_i : public virtual POA_ControlServer::Manager {

	public:
		manager_i(void);
		~manager_i(void);

		void shutdown();
		ControlServer::Session_ptr getSession( const CORBA::WChar * );
	private:
		typedef std::map< std::wstring, boost::shared_ptr< adcontroller::session_i > > session_map_type;
		session_map_type session_list_;
	};

	namespace singleton {
		typedef ACE_Singleton< acewrapper::ORBServant< ::adcontroller::manager_i >, ACE_Recursive_Thread_Mutex > manager;
	}

}
