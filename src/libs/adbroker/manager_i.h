// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <acewrapper/orbservant.h>
#include <map>
#include <string>
#pragma warning (disable: 4996)
#include "adinterface/brokerS.h"
#pragma warning (default: 4996)
#include "logger_i.h"
#include "session_i.h"
#include <boost/smart_ptr.hpp>

namespace adbroker {

    class manager_i : public virtual POA_Broker::Manager {

    public:
        manager_i(void);
        ~manager_i(void);

        void shutdown();
        Broker::Session_ptr getSession( const CORBA::WChar * );
        Broker::Logger_ptr getLogger();
    private:
        typedef std::map< std::wstring, boost::shared_ptr< adbroker::session_i > > session_map_type;
        session_map_type session_list_;
        boost::scoped_ptr< broker::logger_i > logger_i_;
    };

    namespace singleton {
		typedef ACE_Singleton< acewrapper::ORBServant< manager_i >, ACE_Recursive_Thread_Mutex > manager;
	}

}


