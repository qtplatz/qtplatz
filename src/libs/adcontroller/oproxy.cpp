//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "ibroker.h"
#include "oproxy.h"
/*
#include <acewrapper/mutex.hpp>
#include "ibrokermanager.h"
#include "message.h"
#include <acewrapper/timeval.h>
#include <acewrapper/messageblock.h>
#include <iostream>
#include <sstream>
#include <adinterface/eventlog_helper.h>
#include "marshal.hpp"
#include "constants.h"
#include <adportable/configuration.h>
#include <adportable/configloader.h>
*/

using namespace adcontroller;

oProxy::~oProxy()
{
}


oProxy::oProxy( iBroker& t ) : broker_( t )
                             , objref_(false)
							 , objId_(0) 
{
}

void
oProxy::OnUpdateData ( ::CORBA::Long pos )
{
	broker_.observer_update_data( objId_, pos );
}

void
oProxy::OnMethodChanged ( ::CORBA::Long pos )
{
}

void
oProxy::OnEvent ( ::CORBA::ULong event,	::CORBA::Long pos )
{
}

bool
oProxy::connect( const std::wstring& token )
{
	if ( objref_ )
		return impl_->connect( _this(), SignalObserver::Realtime, token.c_str() );
	return false;
}

bool
oProxy::initialize()
{
	return true;
}

void
oProxy::setInstrumentSession( Instrument::Session_ptr iSession )
{
	CORBA::release( iSession_ );
	CORBA::release( impl_ );
    objref_ = false;

	iSession_ = iSession;
	if ( ! CORBA::is_nil( iSession_ ) ) {
		impl_ = iSession_->getObserver();
		if ( ! CORBA::is_nil( impl_.in() ) )
			objref_ = true;
	}
}

void
oProxy::setConfiguration( const adportable::Configuration& c )
{
	config_ = c;
	std::wstring id = c.attribute( L"id" );
	//std::wstring ns = item.attribute( L"ns_name" ); // "tofcontroller.manager"
	//std::wstring type = item.attribute( L"type" ); // "object_ref"
	//std::wstring name = item.name();
}

void
oProxy::objId( unsigned long id )
{
	objId_ = id;
}

unsigned long
oProxy::objId() const
{
	return objId_;
}
