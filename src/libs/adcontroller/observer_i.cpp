//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "observer_i.h"

using namespace adcontroller;

observer_i::observer_i(void)
{
}

observer_i::~observer_i(void)
{
}

::SignalObserver::Description * 
observer_i::getDescription (void)
{
	return 0;
}

::CORBA::Boolean
observer_i::setDescription ( const ::SignalObserver::Description & desc )
{
	return false;
}

::CORBA::Boolean
observer_i::connect ( ::SignalObserver::ObserverEvents_ptr cb
						, ::SignalObserver::eUpdateFrequency frequency
						, const CORBA::WChar * token )
{
	// return task_.connect( cb, frequency, token );
	return true;
}

::CORBA::Boolean
observer_i::isActive (void)
{
	return true;
}

::SignalObserver::Observers *
observer_i::getSiblings (void)
{
	return 0;
}

::CORBA::Boolean
observer_i::addSibling ( ::SignalObserver::Observer_ptr observer )
{
	return false;
}

void
observer_i::uptime ( ::CORBA::ULongLong_out usec )
{
}

::CORBA::Boolean
observer_i::readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer)
{
	return false;
}

::CORBA::WChar *
observer_i::dataInterpreterClsid (void)
{
	return 0;
}
