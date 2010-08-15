// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once


#include <ace/Singleton.h>

#pragma warning (disable : 4996 )
# include <adinterface/instrumentS.h>
#pragma warning (default : 4996 )

#include <acewrapper/orbservant.h>
#include <boost/smart_ptr.hpp>

namespace tofcontroller {

	class i8tManager_i;
    class i8tTask;

	namespace singleton {
		typedef ACE_Singleton< acewrapper::ORBServant< i8tManager_i >, ACE_Recursive_Thread_Mutex > i8tManager_i;
	}

	class i8tManager_i : public virtual POA_Instrument::Session {
		i8tManager_i(void);
		~i8tManager_i(void);
		friend singleton::i8tManager_i;
		friend acewrapper::ORBServant< i8tManager_i >;
	public:

		// POA_Instrument::Session
		CORBA::WChar * software_revision (void);
		CORBA::Boolean setConfiguration( const CORBA::WChar * );
		CORBA::Boolean configComplete();
		CORBA::Boolean connect( Receiver_ptr receiver, const CORBA::WChar * toke );
		CORBA::Boolean disconnect ( Receiver_ptr receiver );
		CORBA::ULong get_status (void);
		CORBA::Boolean initialize (void);
		CORBA::Boolean shutdown (void);
		CORBA::Boolean echo (const char * msg);
		CORBA::Boolean shell (const char * cmdline);
		CORBA::Boolean prepare_for_run ( ControlMethod::Method_ptr m);
		CORBA::Boolean push_back ( SampleBroker::SampleSequence_ptr s);
		CORBA::Boolean event_out ( CORBA::ULong event);
		CORBA::Boolean start_run (void);
		CORBA::Boolean suspend_run (void);
		CORBA::Boolean resume_run (void);
		CORBA::Boolean stop_run (void);
	private:
		boost::scoped_ptr< i8tTask > pTask_;
	};


}
