/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "traceobserver_i.hpp"
#include "toftask.hpp"
#include "constants.h"
#include <tofspectrometer/constants.hpp>
#include <adportable/scope_timer.hpp>
#include <tofinterface/tofprocessed.hpp>
#include <tofinterface/serializer.hpp>

using namespace tofservant;

traceObserver_i::traceObserver_i() : objId_(0)
{
    desc_.trace_method = SignalObserver::eTRACE_TRACE;
    desc_.spectrometer = SignalObserver::eMassSpectrometer;
    desc_.axis_x_label = CORBA::wstring_dup( L"Time(min)" );
    desc_.axis_y_label = CORBA::wstring_dup( L"Intensity" );
    desc_.axis_x_decimals = 2;
    desc_.axis_y_decimals = 0;
}

traceObserver_i::~traceObserver_i(void)
{
}

::SignalObserver::Description * 
traceObserver_i::getDescription (void)
{
    // adportable::scope_timer<>( "traceObserver_i::getDescription: ");
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
    // adportable::scope_timer<>( "traceObserver_i::assign_objId: ");
    objId_ = oid;
    std::cerr << "traceObserver_i::assign_objId(" << oid << ")" << std::endl;
}

::CORBA::Boolean
traceObserver_i::connect ( ::SignalObserver::ObserverEvents_ptr cb
						, ::SignalObserver::eUpdateFrequency frequency
						, const CORBA::Char * token )
{
    // adportable::scope_timer<>( "traceObserver_i::connect: ");
    return toftask::instance()->connect( cb, frequency, token );
}

::CORBA::Boolean
traceObserver_i::disconnect( ::SignalObserver::ObserverEvents_ptr cb )
{
    return toftask::instance()->disconnect( cb );
}


::CORBA::Boolean
traceObserver_i::isActive (void)
{
	return true;
}

::SignalObserver::Observers *
traceObserver_i::getSiblings (void)
{
    std::cerr << "traceObserver_i::getSiblings : no siblings" << std::endl;

    SignalObserver::Observers_var vec( new SignalObserver::Observers );
    return vec._retn();  // zero length
}

::CORBA::Boolean
traceObserver_i::addSibling ( ::SignalObserver::Observer_ptr observer )
{
    (void)( observer );
    return false;
}

::SignalObserver::Observer *
traceObserver_i::findObserver( CORBA::ULong objId, CORBA::Boolean recursive )
{
    (void)( objId );
    (void)( recursive );
    return 0;  // this class never has sibling
}


void
traceObserver_i::uptime ( ::CORBA::ULongLong_out usec )
{
    (void)( usec );
}

void
traceObserver_i::uptime_range( ::CORBA::ULongLong_out oldest, ::CORBA::ULongLong_out newest )
{
    (void)( oldest );
    (void)( newest );
}

::CORBA::Boolean
traceObserver_i::readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    
    if ( pos < 0 )
        pos = fifo_.back();
    
    if ( ! fifo_.empty() && ( fifo_.front() <= pos && pos <= fifo_.back() ) ) {
        std::deque< cache_item >::iterator it = std::lower_bound( fifo_.begin(), fifo_.end(), pos );
        
        if ( it != fifo_.end() ) {
            SignalObserver::DataReadBuffer_var rb = new SignalObserver::DataReadBuffer;
            rb->pos = it->pos_;
            rb->events = it->meta_.wellKnownEvents;
            rb->uptime = it->meta_.uptime;
            rb->ndata = std::distance( it, fifo_.end() );

            std::vector< tofinterface::tofProcessedData > ar;
			std::for_each( it, fifo_.end(), [&ar]( const cache_item& item ){
					ar.push_back( item.data_ );
				});
			std::string device;
			tofinterface::serializer::serialize( ar, device );
			rb->xdata.length( device.size() );
			std::copy( device.begin(), device.end(), rb->xdata.get_buffer() );
            dataReadBuffer = rb._retn();
            return true;
        }
    }

    return false;
}

::CORBA::WChar *
traceObserver_i::dataInterpreterClsid (void)
{
    return CORBA::wstring_dup( tofspectrometer::constants::dataInterpreter::spectrometer::name() ); // L"TOF"
}

CORBA::Long
traceObserver_i::posFromTime( CORBA::ULongLong usec )
{
    (void)usec;
    return (-1);
}

/////////////////////////
void
traceObserver_i::push_back( long pos
			    , const tofinterface::tofProcessedData& data
			    , const tofinterface::TraceMetadata& meta )
{
    unsigned long prevEvents = 0;
    do {
        std::lock_guard< std::mutex > lock( mutex_ );

        if ( ! fifo_.empty() )
            prevEvents = fifo_.back().meta_.wellKnownEvents;

        fifo_.push_back( cache_item( pos, data, meta ) );
        if ( fifo_.size() > 64 )
            fifo_.pop_front();
    } while(0);

    toftask::instance()->observer_fire_on_update_data( this->objId_, pos ); 

    if ( prevEvents != meta.wellKnownEvents )
        toftask::instance()->observer_fire_on_event( this->objId_, meta.wellKnownEvents, pos );

}

/////////////////////// cache item ////////////////////////////

traceObserver_i::cache_item::~cache_item()
{
}

traceObserver_i::cache_item::cache_item( long pos
                                        , const tofinterface::tofProcessedData& data
                                        , const tofinterface::TraceMetadata& meta ) : pos_( pos )
                                                                                 , data_( data )
                                                                                 , meta_( meta )  
{
}

traceObserver_i::cache_item::cache_item( const cache_item& t ) : pos_( t.pos_ )
                                                               , data_( t.data_ )
                                                               , meta_( t.meta_ )
{
}

traceObserver_i::cache_item::operator long() const
{
    return pos_;
}

