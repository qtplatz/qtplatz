// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <adinterface/signalobserverS.h>
//#include <adinterface/instrumentC.h>
//#include <adportable/configuration.h>
//#include <string>
#include <boost/noncopyable.hpp>

namespace adcontroller {

    class iBroker;

	class oProxy : public POA_SignalObserver::ObserverEvents, boost::noncopyable {
    public:
		~oProxy();
		oProxy( iBroker& );

		// POA_SignalObserver::ObserverEvents implementation
		virtual void OnUpdateData (::CORBA::Long pos);
		virtual void OnMethodChanged (::CORBA::Long pos);
		virtual void OnEvent (::CORBA::ULong event,	::CORBA::Long pos);

		// oProxy implementation

    private:
		iBroker& broker_;
    };
    
}
