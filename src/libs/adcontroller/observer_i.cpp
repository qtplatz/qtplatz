//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "observer_i.h"

namespace adcontroller {

	namespace internal {

		struct observer_events_data {
			bool operator == ( const observer_events_data& ) const;
			bool operator == ( const SignalObserver::ObserverEvents_ptr ) const;
			SignalObserver::ObserverEvents_var events_;
			observer_events_data() {}
			observer_events_data( const observer_events_data& t ) : events_( t.events_ ) {}
        };

		struct sibling_data {
			SignalObserver::Observer_var observer_;
            unsigned long objId_;
			sibling_data() : objId_(0) {}
			sibling_data( const sibling_data& t ) : observer_( t.observer_ ), objId_( t.objId_ ) {}
        };
      
	}
}

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

CORBA::ULong
observer_i::objId()
{
	return 0;
}

void
observer_i::assign_objId( CORBA::ULong oid )
{
	return;  // master object should always be id = 0;
}


::CORBA::Boolean
observer_i::connect ( ::SignalObserver::ObserverEvents_ptr cb
						, ::SignalObserver::eUpdateFrequency frequency
						, const CORBA::WChar * token )
{
	using namespace adcontroller::internal;

    observer_events_data data;
    data.events_ = cb;
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
	internal::sibling_data data;
    data.observer_ = SignalObserver::Observer::_duplicate( observer );
	data.objId_ = data.observer_->objId();
	sibling_set_.push_back( data );
	return true;
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

void
observer_i::invoke_update_data( unsigned long objid, long pos )
{
	ACE_UNUSED_ARG( objid );
    ACE_UNUSED_ARG( pos );
    // this method will be used when TIVO cache is implemented in future
}

void
observer_i::invoke_update_method( unsigned long objid, long pos )
{
	ACE_UNUSED_ARG( objid );
    ACE_UNUSED_ARG( pos );
    // this method will be used when TIVO cache is implemented in future
}

void
observer_i::invoke_update_event( unsigned long objid, long pos, unsigned long event )
{
	ACE_UNUSED_ARG( objid );
    ACE_UNUSED_ARG( pos );
    ACE_UNUSED_ARG( event );
    // this method will be used when TIVO cache is implemented in future
}

////////////////////////////////////////////

bool
internal::observer_events_data::operator == ( const internal::observer_events_data& t ) const
{
	return events_->_is_equivalent( t.events_.in() );
}

bool
internal::observer_events_data::operator == ( const SignalObserver::ObserverEvents_ptr t ) const
{
	return events_->_is_equivalent( t );
}
