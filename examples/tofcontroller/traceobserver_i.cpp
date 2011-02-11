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
traceObserver_i::findObserver( CORBA::ULong /* objId */, CORBA::Boolean /* recursive */)
{
    // this class does not has any siblings
    return 0;
}


void
traceObserver_i::uptime ( ::CORBA::ULongLong_out usec )
{
	ACE_UNUSED_ARG( usec );
} 

::CORBA::Boolean
traceObserver_i::readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer )
{
    acewrapper::scoped_mutex_t<> lock( mutex_ );

    if ( pos < 0 )
        pos = fifo_.back();

    if ( !fifo_.empty() && ( fifo_.front() <= pos && pos <= fifo_.back() ) ) {

        std::deque< cache_item >::iterator it = std::lower_bound( fifo_.begin(), fifo_.end(), pos );

        if ( it != fifo_.end() ) {
            SignalObserver::DataReadBuffer_var rb = new SignalObserver::DataReadBuffer;
            
            rb->pos = it->pos_;
            const TOFInstrument::TraceDescriptor& desc = it->desc_;

            rb->events = desc.wellKnownEvents;
            rb->ndata = std::distance( it, fifo_.end() );

            TOFInstrument::SpectrumProcessedDataArray ar;
            ar.length( rb->ndata );
            for ( size_t i = 0 ; it != fifo_.end(); ++it, ++i ) {
                const TOFInstrument::SpectrumProcessedData& d = it->data_;
                ar[i] = d;
            }

            rb->data <<= ar;

            dataReadBuffer = rb._retn();
            return true;
        }
    }
	return false;
}

::CORBA::WChar *
traceObserver_i::dataInterpreterClsid (void)
{
    return CORBA::wstring_dup( L"tofSpectrometer" );  // same id as tofObserver
}

CORBA::Long
traceObserver_i::posFromTime( CORBA::ULongLong usec )
{
    return -1;
}

void
traceObserver_i::push_trace_data( long pos, const TOFInstrument::SpectrumProcessedData& data, const TOFInstrument::TraceDescriptor& desc )
{
    unsigned long prevEvents(0);
    do {
        acewrapper::scoped_mutex_t<> lock( mutex_ );
        // get previous events
        if ( ! fifo_.empty() )
            prevEvents = fifo_.back().desc_.wellKnownEvents;
        // push data 
        fifo_.push_back( cache_item( pos, data, desc ) );
        // erase obsolete data
        if ( fifo_.size() > 512 )
            fifo_.pop_front();
    } while(0);

    task_.observer_fire_on_update_data( objId_, pos );
    if ( prevEvents != desc.wellKnownEvents )
        task_.observer_fire_on_event( objId_, desc.wellKnownEvents, pos );
}

/////////////////////// cache item ////////////////////////////

traceObserver_i::cache_item::~cache_item()
{
}

traceObserver_i::cache_item::cache_item( long pos
                                        , const TOFInstrument::SpectrumProcessedData& data
                                        , const TOFInstrument::TraceDescriptor& desc ) 
                                        : pos_(pos)
                                        , data_(data)
                                        , desc_(desc) 
{
}

traceObserver_i::cache_item::cache_item( const cache_item& t ) : pos_(t.pos_)
                                                               , data_(t.data_)
                                                               , desc_( t.desc_ )
{
}

traceObserver_i::cache_item::operator long() const
{
    return pos_;
}

