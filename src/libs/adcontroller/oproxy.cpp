//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "ibroker.h"
#include "oproxy.h"

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
	broker_.observer_update_data( objId_, pos );
}

void
oProxy::OnEvent ( ::CORBA::ULong event,	::CORBA::Long pos )
{
	broker_.observer_update_event( objId_, pos, event );
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

bool
oProxy::setInstrumentSession( Instrument::Session_ptr iSession )
{
    objref_ = false;

	CORBA::release( iSession_ );
	CORBA::release( impl_ );

	iSession_ = iSession;
	if ( ! CORBA::is_nil( iSession_ ) ) {
		impl_ = iSession_->getObserver();
		if ( ! CORBA::is_nil( impl_.in() ) ) {
			objref_ = true;
			impl_->assign_objId( objId_ );
			return true;
		}
	}
}

size_t
oProxy::populateObservers( unsigned long objid )
{
	size_t nsize = 0;
	SignalObserver::Observers_var vec = impl_->getSiblings();
	if ( ( vec.ptr() != 0) && ( nsize = vec->length() ) > 0 ) {
		for ( size_t i = 0; i < nsize; ++i )
			vec[i]->assign_objId( ++objid );
	}
	return nsize;
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

unsigned long
oProxy::objId() const
{
	return objId_;
}

void
oProxy::objId( unsigned long objid )
{
	objId_ = objid;
}

SignalObserver::Observer_ptr
oProxy::getObject()
{
	return impl_.in();
}