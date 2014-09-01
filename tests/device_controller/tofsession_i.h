// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#pragma warning (disable : 4996 )
# include "tofcontrollerS.h"
#pragma warning (default : 4996 )

#include <ace/Singleton.h>
#include <acewrapper/orbservant.h>
#include <boost/smart_ptr.hpp>

namespace tofcontroller {

	class tofSession_i;
    class Task;

	namespace singleton {
		typedef ACE_Singleton< acewrapper::ORBServant< tofSession_i >, ACE_Recursive_Thread_Mutex > tofSession_i;
	}

	class tofSession_i : public virtual POA_TOFInstrument::TofSession {
		tofSession_i(void);
		~tofSession_i(void);
		friend singleton::tofSession_i;
		friend acewrapper::ORBServant< tofSession_i >;
	public:

        // POA_TOFInstrument::TofSession
		CORBA::WChar * tof_software_revision (void);
		void tof_debug( const CORBA::WChar * text, const CORBA::WChar * key );
		void setADConfigurations( const TOFInstrument::ADConfigurations& config );
		bool getADConfigurations( TOFInstrument::ADConfigurations_out config );

		void setAnalyzerDeviceData( const TOFInstrument::AnalyzerDeviceData& data );
        TOFInstrument::AnalyzerDeviceData * getAnalyzerDeviceData();

		// POA_Instrument::Session
		CORBA::WChar * software_revision (void);
		CORBA::Boolean setConfiguration( const CORBA::WChar * );
		CORBA::Boolean configComplete();
		CORBA::Boolean connect( Receiver_ptr receiver, const CORBA::WChar * toke );
		CORBA::Boolean disconnect ( Receiver_ptr receiver );
		CORBA::ULong get_status (void);
		CORBA::Boolean initialize (void);
		SignalObserver::Observer_ptr getObserver(void);
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
		boost::scoped_ptr< Task > pTask_;
		Instrument::eInstStatus status_current_;
	};
}
