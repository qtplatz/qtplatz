//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#include "traceobserver_i.h"
#include "task.h"

using namespace tofcontroller;

traceObserver_i::traceObserver_i( Task& t ) : task_(t)
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
traceObserver_i::disconnect ( ::SignalObserver::ObserverEvents_ptr cb )
{
	return task_.disconnect( cb );
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
    ACE_UNUSED_ARG( observer );
	return false;
}

::SignalObserver::Observer *
traceObserver_i::findObserver( CORBA::ULong objId, CORBA::Boolean recursive )
{
    ACE_UNUSED_ARG( objId );
	ACE_UNUSED_ARG( recursive );
    return 0;  // this class never has sibling
}


void
traceObserver_i::uptime ( ::CORBA::ULongLong_out usec )
{
	ACE_UNUSED_ARG( usec );
}

::CORBA::Boolean
traceObserver_i::readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer)
{
	ACE_UNUSED_ARG( pos );
	ACE_UNUSED_ARG( dataReadBuffer );
	return false;
}

::CORBA::WChar *
traceObserver_i::dataInterpreterClsid (void)
{
	return 0;
}

::CORBA::Long
traceObserver_i::posFromTime( CORBA::ULongLong usec )
{
    (void)usec;
    return -1;
}
