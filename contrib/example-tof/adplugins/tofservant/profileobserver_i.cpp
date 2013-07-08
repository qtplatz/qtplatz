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

#include "profileobserver_i.hpp"
#include "toftask.hpp"
#include "constants.h"
#include "logger.hpp"
#include <tofspectrometer/constants.hpp>
#include <tofinterface/signalC.h>
#include <adportable/debug.hpp>
#include <adportable/scope_timer.hpp>

using namespace tofservant;

//////////////////////////////////

profileObserver_i::profileObserver_i() : objId_(0)
{
    desc_.trace_method = SignalObserver::eTRACE_SPECTRA;
    desc_.spectrometer = SignalObserver::eMassSpectrometer;
    desc_.trace_id = CORBA::wstring_dup( L"MS.PROFILE" );
    desc_.trace_display_name = CORBA::wstring_dup( L"TOF MS Spectrum" );
    desc_.axis_x_label = CORBA::wstring_dup( L"m/z" );
    desc_.axis_y_label = CORBA::wstring_dup( L"Intens" );
    desc_.axis_x_decimals = 2;
    desc_.axis_y_decimals = 0;
}

profileObserver_i::~profileObserver_i(void)
{
}

::SignalObserver::Description * 
profileObserver_i::getDescription (void)
{
    SignalObserver::Description_var var( new SignalObserver::Description( desc_ ) );
    return var._retn();
}

::CORBA::Boolean
profileObserver_i::setDescription ( const ::SignalObserver::Description & desc )
{
    desc_ = desc;
    return true;
}

CORBA::ULong
profileObserver_i::objId()
{
    return objId_;
}

void
profileObserver_i::assign_objId( CORBA::ULong oid )
{
    // adportable::scope_timer<>( "profileObserver_i::assignObjeId: ");
    objId_ = oid;
    std::cerr << "profileObserver_i::assign_objId(" << oid << ")" << std::endl;
}

::CORBA::Boolean
profileObserver_i::connect ( ::SignalObserver::ObserverEvents_ptr cb
                             , ::SignalObserver::eUpdateFrequency frequency
                             , const CORBA::Char * token )
{
    // adportable::scope_timer<>( "profileObserver_i::connect: ");
    Logger( L"profileObserver_i::connect from %1% on id=%2%(%3%)" ) % token % objId_ % this;
    return toftask::instance()->connect( cb, frequency, token );
}

::CORBA::Boolean
profileObserver_i::disconnect( ::SignalObserver::ObserverEvents_ptr cb )
{
    return toftask::instance()->disconnect( cb );
}

::CORBA::Boolean
profileObserver_i::isActive (void)
{
    return true;
}

::SignalObserver::Observers *
profileObserver_i::getSiblings (void)
{
    // adportable::scope_timer<>( "profileObserver_i::getSiblings: ");

    SignalObserver::Observers_var vec( new SignalObserver::Observers );
    vec->length( siblings_.size() );

    for ( size_t i = 0; i < siblings_.size(); ++i )
	(*vec)[i] = SignalObserver::Observer::_duplicate( siblings_[i].in() );

    return vec._retn();
}

::SignalObserver::Observer *
profileObserver_i::findObserver( CORBA::ULong objId, CORBA::Boolean recursive )
{
    // adportable::scope_timer<>( "profileObserver_i::findObserver: ");

    for ( sibling_vector_type::iterator it = siblings_.begin(); it != siblings_.end(); ++it ) {
        if ( (*it)->objId() == objId )
            return SignalObserver::Observer::_duplicate( it->in() );
    }

    if ( recursive ) {
        ::SignalObserver::Observer * pres = 0;
        for ( sibling_vector_type::iterator it = siblings_.begin(); it != siblings_.end(); ++it ) {
            if ( ( pres = (*it)->findObserver( objId, true ) ) )
                return pres;
        }
    }
    return 0;
}

::CORBA::Boolean
profileObserver_i::addSibling ( ::SignalObserver::Observer_ptr observer )
{
    adportable::scope_timer<>( "profileObserver_i::addSibling: ");

    ::SignalObserver::Observer_var var = SignalObserver::Observer::_duplicate( observer );
    siblings_.push_back( var );
    return true;
}

void
profileObserver_i::uptime ( ::CORBA::ULongLong_out usec )
{
    ACE_UNUSED_ARG( usec );
}

void
profileObserver_i::uptime_range( ::CORBA::ULongLong_out oldest, ::CORBA::ULongLong_out newest )
{
    ACE_UNUSED_ARG( oldest );
    ACE_UNUSED_ARG( newest );
}

::CORBA::Boolean
profileObserver_i::readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer )
{
    (void)dataReadBuffer;

    std::cout << "###################### readData pos = " << pos << " ###########################" << std::endl;

    std::lock_guard< std::mutex > lock( mutex_ );

    if ( pos < 0 )
        pos = fifo_.back();

    if ( !fifo_.empty() && ( fifo_.front() <= pos && pos <= fifo_.back() ) ) {
        int noffs = pos - fifo_.front().pos_;
        if ( noffs < 0 || unsigned(noffs) >= fifo_.size() )
            return false;
        cache_item& d = fifo_[ noffs ];
        if ( d.pos_ != pos ) { // in case if some data lost
            std::deque< cache_item >::iterator it = std::lower_bound( fifo_.begin(), fifo_.end(), pos );
            if ( it != fifo_.end() )
                d = *it;
            else
                return false;
        }

		if ( d.data_ ) {

            SignalObserver::DataReadBuffer_var res = new SignalObserver::DataReadBuffer;

            res->data <<= *d.data_;
            //res->method <<= method;
            res->uptime = d.data_->clockTimeStamp;
            res->pos    = d.data_->sequenceNumber;
            res->events = d.data_->wellKnownEvents;
            res->array.length( 0 );

            dataReadBuffer = res._retn();

            return true;
        }
    }
	return false;
}

::CORBA::WChar *
profileObserver_i::dataInterpreterClsid (void)
{
    return CORBA::wstring_dup( tofspectrometer::constants::dataInterpreter::spectrometer::name() ); // L"TOF"
}

CORBA::Long
profileObserver_i::posFromTime( CORBA::ULongLong usec )
{
    (void)usec;
    return (-1);
}

void
profileObserver_i::push_profile_data( std::shared_ptr< TOFSignal::tofDATA >& data, long npos, unsigned long wellKnownEvents )
{
	toftask& task = *toftask::instance();

    if ( ! data ) {
        adportable::debug() << "profileObserver_i::push_profile_data -- not MB_TOF_DATA";
        return;
    }
    unsigned long prevEvents = 0;
    do {
        std::lock_guard< std::mutex >lock( mutex_ );
        if ( ! fifo_.empty() )
            prevEvents = fifo_.back().wellKnownEvents_;
        fifo_.push_back( cache_item( npos, data, wellKnownEvents ) );

        if ( fifo_.size() > 64 )
            fifo_.pop_front();
    } while(0);

    task.observer_fire_on_update_data( objId_, npos );

    if ( prevEvents != wellKnownEvents )
        task.observer_fire_on_event( objId_, wellKnownEvents, npos );
}

/////////////////////// cache item ////////////////////////////

profileObserver_i::cache_item::~cache_item()
{
}

profileObserver_i::cache_item::cache_item( long pos
										   , std::shared_ptr< TOFSignal::tofDATA >&  data
                                           , unsigned long event ) : pos_(pos)
                                                                   , wellKnownEvents_(event)
                                                                   , data_( data )
{
}

profileObserver_i::cache_item::cache_item( const cache_item& t ) : pos_(t.pos_)
                                                             , wellKnownEvents_(t.wellKnownEvents_)
                                                             , data_( t.data_ )
{
}

profileObserver_i::cache_item::operator long() const
{
    return pos_;
}

