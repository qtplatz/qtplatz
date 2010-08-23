//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "tofobserver_i.h"

using namespace tofcontroller;

tofObserver_i::tofObserver_i(void)
{
}

tofObserver_i::~tofObserver_i(void)
{
}

::SignalObserver::Description * 
tofObserver_i::getDescription (void)
{
	return 0;
}

::CORBA::Boolean
tofObserver_i::setDescription ( const ::SignalObserver::Description & desc )
{
	return false;
}

::CORBA::Boolean
tofObserver_i::connect ( ::SignalObserver::ObserverEvents_ptr cb,	::SignalObserver::eUpdateFrequency frequency)
{
	return false;
}

::CORBA::Boolean
tofObserver_i::isActive (void)
{
	return false;
}

::SignalObserver::Observers *
tofObserver_i::getSiblings (void)
{
	return 0;
}

::CORBA::Boolean
tofObserver_i::addSibling ( ::SignalObserver::Observer_ptr observer )
{
	return false;
}

void
tofObserver_i::uptime ( ::CORBA::ULongLong_out usec )
{
}

::CORBA::Boolean
tofObserver_i::readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer)
{
	return false;
}

::CORBA::WChar *
tofObserver_i::dataInterpreterClsid (void)
{
	return 0;
}

