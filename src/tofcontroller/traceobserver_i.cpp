//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "traceobserver_i.h"
#include "toftask.h"

using namespace tofcontroller;

traceObserver_i::traceObserver_i( TOFTask& t ) : task_(t)
                                               , objId_(0)
{
	desc_.trace_method = SignalObserver::eTRACE_TRACE;
	// desc_.trace_id = CORBA::wstring_dup( L"MS.PROFILE" );
	// desc_.trace_display_name = CORBA::wstring_dup( L"Spectrum" );
	// desc_.axis_x_label = CORBA::wstring_dup( L"m/z" );
	// desc_.axis_y_label = CORBA::wstring_dup( L"Intens" );
	desc_.axis_x_decimals = 2;
	desc_.axis_y_decimals = 0;
}

traceObserver_i::~traceObserver_i(void)
{
}

::SignalObserver::Description * 
traceObserver_i::getDescription (void)
{
	SignalObserver::Description_var var( new SignalObserver::Description( desc_ ) );
    return var._retn();
}

::CORBA::Boolean
traceObserver_i::setDescription ( const ::SignalObserver::Description & desc )
{
    desc_ = desc;
	return true;
}

CORBA::ULong
traceObserver_i::objId()
{
	return objId_;
}

void
traceObserver_i::assign_objId( CORBA::ULong oid )
{
	objId_ = oid;
}

::CORBA::Boolean
traceObserver_i::connect ( ::SignalObserver::ObserverEvents_ptr cb
						, ::SignalObserver::eUpdateFrequency frequency
						, const CORBA::WChar * token )
{
	return task_.connect( cb, frequency, token );
}

::CORBA::Boolean
traceObserver_i::isActive (void)
{
	return true;
}

::SignalObserver::Observers *
traceObserver_i::getSiblings (void)
{
	return 0;
}

::CORBA::Boolean
traceObserver_i::addSibling ( ::SignalObserver::Observer_ptr observer )
{
	return false;
}

void
traceObserver_i::uptime ( ::CORBA::ULongLong_out usec )
{
}

::CORBA::Boolean
traceObserver_i::readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer)
{
	return false;
}

::CORBA::WChar *
traceObserver_i::dataInterpreterClsid (void)
{
	return 0;
}

