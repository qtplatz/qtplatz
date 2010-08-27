// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <adinterface/signalobserverS.h>
#include <adinterface/instrumentC.h>
#include <adportable/configuration.h>
//#include <string>
#include <boost/noncopyable.hpp>

namespace adcontroller {

    class iBroker;
    class iProxy;

	class oProxy : public POA_SignalObserver::ObserverEvents, boost::noncopyable {
    public:
		~oProxy();
		oProxy( iBroker& );

		// POA_SignalObserver::ObserverEvents implementation
		virtual void OnUpdateData (::CORBA::Long pos);
		virtual void OnMethodChanged (::CORBA::Long pos);
		virtual void OnEvent (::CORBA::ULong event,	::CORBA::Long pos);

		// oProxy implementation
        bool initialize();
		bool connect( const std::wstring& token );
		void setInstrumentSession( Instrument::Session_ptr p );
        void setConfiguration( const adportable::Configuration& );

        void objId( unsigned long );
		unsigned long objId() const;

    private:
		bool objref_;
		unsigned long objId_;
		iBroker& broker_;
		Instrument::Session_var iSession_;
		SignalObserver::Observer_var impl_;
        adportable::Configuration config_;
    };
    
}
