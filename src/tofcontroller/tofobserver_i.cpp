//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "tofobserver_i.h"
#include "toftask.h"

using namespace tofcontroller;

tofObserver_i::tofObserver_i( TOFTask& t ) : task_(t)
                                           , objId_(0)
{
	desc_.trace_method = SignalObserver::eTRACE_SPECTRA;
	desc_.trace_id = CORBA::wstring_dup( L"MS.PROFILE" );
	desc_.trace_display_name = CORBA::wstring_dup( L"Spectrum" );
	desc_.axis_x_label = CORBA::wstring_dup( L"m/z" );
	desc_.axis_y_label = CORBA::wstring_dup( L"Intens" );
	desc_.axis_x_decimals = 2;
	desc_.axis_y_decimals = 0;
}

tofObserver_i::~tofObserver_i(void)
{
}

::SignalObserver::Description * 
tofObserver_i::getDescription (void)
{
	SignalObserver::Description_var var( new SignalObserver::Description( desc_ ) );
    return var._retn();
}

::CORBA::Boolean
tofObserver_i::setDescription ( const ::SignalObserver::Description & desc )
{
    desc_ = desc;
	return true;
}

CORBA::ULong
tofObserver_i::objId()
{
	return objId_;
}

void
tofObserver_i::assign_objId( CORBA::ULong oid )
{
	objId_ = oid;
}

::CORBA::Boolean
tofObserver_i::connect ( ::SignalObserver::ObserverEvents_ptr cb
						, ::SignalObserver::eUpdateFrequency frequency
						, const CORBA::WChar * token )
{
	return task_.connect( cb, frequency, token );
}

::CORBA::Boolean
tofObserver_i::isActive (void)
{
	return true;
}

::SignalObserver::Observers *
tofObserver_i::getSiblings (void)
{
	SignalObserver::Observers_var vec( new SignalObserver::Observers );
	vec->length( siblings_.size() );

	for ( size_t i = 0; i < siblings_.size(); ++i )
		(*vec)[i] = SignalObserver::Observer::_duplicate( siblings_[i].in() );

	return vec._retn();
}

::CORBA::Boolean
tofObserver_i::addSibling ( ::SignalObserver::Observer_ptr observer )
{
	::SignalObserver::Observer_var var = SignalObserver::Observer::_duplicate( observer );
    siblings_.push_back( var );
	return true;
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

