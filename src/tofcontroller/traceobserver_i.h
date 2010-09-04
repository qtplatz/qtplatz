// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#pragma warning (disable : 4996 )
# include "tofcontrollerS.h"
# include <adinterface/signalobserverS.h>
#pragma warning (default : 4996 )

namespace tofcontroller {

    class TOFTask;

	class traceObserver_i : public virtual POA_SignalObserver::Observer {
	public:
		traceObserver_i( TOFTask& );
		~traceObserver_i(void);

		virtual ::SignalObserver::Description * getDescription (void);
		virtual ::CORBA::Boolean setDescription ( const ::SignalObserver::Description & desc );
		virtual ::CORBA::ULong objId();
		virtual void assign_objId( CORBA::ULong oid );
		virtual ::CORBA::Boolean connect( ::SignalObserver::ObserverEvents_ptr cb
			                             , ::SignalObserver::eUpdateFrequency frequency
										 , const CORBA::WChar * );
		virtual ::CORBA::Boolean isActive (void);
		virtual ::SignalObserver::Observers * getSiblings (void);
		virtual ::CORBA::Boolean addSibling ( ::SignalObserver::Observer_ptr observer);
        virtual ::SignalObserver::Observer * findObserver( CORBA::ULong objId, CORBA::Boolean recursive );
		virtual void uptime ( ::CORBA::ULongLong_out usec );
		virtual ::CORBA::Boolean readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer);
		virtual ::CORBA::WChar * dataInterpreterClsid (void);
	private:
		TOFTask & task_;
        unsigned long objId_;
		SignalObserver::Description desc_;
	};

}

