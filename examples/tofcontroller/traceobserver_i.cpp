//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "traceobserver_i.h"
#include "toftask.h"
#include <acewrapper/mutex.hpp>

using namespace tofcontroller;

traceObserver_i::traceObserver_i( TOFTask& t ) : task_(t)
                                               , objId_(0)
{
/*
	desc_.trace_method = SignalObserver::eTRACE_TRACE;
    desc_.spectrometer = SignalObserver::eMassSpectrometer;
    desc_.trace_id = CORBA::wstring_dup( L"MS.TIC" );
	desc_.trace_display_name = CORBA::wstring_dup( L"TIC" );
	desc_.axis_x_label = CORBA::wstring_dup( L"min" );
	desc_.axis_y_label = CORBA::wstring_dup( L"intensity" );
	desc_.axis_x_decimals = 2;
	desc_.axis_y_decimals = 0;
*/
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
    return CORBA::wstring_dup( L"tofSpectrometer" );  // same id as tofObserver
}

void
traceObserver_i::push_trace_data( long pos, double value, unsigned long events )
{
    unsigned long prevEvents(0);
    do {
        acewrapper::scoped_mutex_t<> lock( mutex_ );
        // get previous events
        if ( ! fifo_.empty() )
            prevEvents = fifo_.back().wellKnownEvents_;
        // push data 
        fifo_.push_back( cache_item( pos, value, events ) );
        // erase obsolete data
        if ( fifo_.size() > 64 )
            fifo_.pop_front();
    } while(0);

    task_.observer_fire_on_update_data( objId_, pos );
    if ( prevEvents != events )
        task_.observer_fire_on_event( objId_, events, pos );
}

/////////////////////// cache item ////////////////////////////

traceObserver_i::cache_item::~cache_item()
{
}

traceObserver_i::cache_item::cache_item( long pos, double value
                                        , unsigned long events ) : pos_(pos)
                                                                 , value_(value)
                                                                 , wellKnownEvents_(events)
{
}

traceObserver_i::cache_item::cache_item( const cache_item& t ) : pos_(t.pos_)
                                                               , value_(t.value_)
                                                               , wellKnownEvents_(t.wellKnownEvents_)
{
}

traceObserver_i::cache_item::operator long() const
{
    return pos_;
}

