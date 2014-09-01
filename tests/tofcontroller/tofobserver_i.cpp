// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "tofobserver_i.h"
#include "toftask.h"
#include <acewrapper/mutex.hpp>

using namespace tofcontroller;

//////////////////////////////////

tofObserver_i::tofObserver_i( TOFTask& t ) : task_(t)
                                           , objId_(0)
{
	desc_.trace_method = SignalObserver::eTRACE_SPECTRA;
    desc_.spectrometer = SignalObserver::eMassSpectrometer;
	desc_.trace_id = CORBA::wstring_dup( L"MS.PROFILE" );
    desc_.trace_display_name = CORBA::wstring_dup( L"MS-Spectrum" );
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
tofObserver_i::disconnect ( ::SignalObserver::ObserverEvents_ptr cb )
{
	return task_.disconnect( cb );
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

::SignalObserver::Observer *
tofObserver_i::findObserver( CORBA::ULong objId, CORBA::Boolean recursive )
{
    for ( sibling_vector_type::iterator it = siblings_.begin(); it != siblings_.end(); ++it ) {
        if ( (*it)->objId() == objId )
            return SignalObserver::Observer::_duplicate( it->in() );
    }

    if ( recursive ) {
        ::SignalObserver::Observer * pres = 0;
        for ( sibling_vector_type::iterator it = siblings_.begin(); it != siblings_.end(); ++it ) {
            if ( pres = (*it)->findObserver( objId, true ) )
                return pres;
        }
    }
    return 0;
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
    ACE_UNUSED_ARG( usec );
}

::CORBA::Boolean
tofObserver_i::readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer )
{
    acewrapper::scoped_mutex_t<> lock( mutex_ );

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

        TAO_InputCDR cdr( d.mb_->rd_ptr(), sizeof(unsigned long) * 32 );

        TOFInstrument::AveragerData data;
        CORBA::ULong clsid;
        cdr >> clsid;
        assert( clsid == TOFConstants::ClassID_ProfileData );
        cdr >> data;

        SignalObserver::AveragerData avgr;
        avgr.nbrSamples = data.nbrSamples;
        avgr.nbrAverage = data.nbrAvrg;
        avgr.sampInterval = data.sampInterval;
        avgr.startDelay = data.startDelay;
        avgr.wellKnownEvents = data.wellKnownEvents;
        avgr.uptime = data.uptime;
        avgr.timeSinceInject = data.timeSinceInject;  // todo: keep most recent injection time, and subtract from this value

        SignalObserver::DataReadBuffer_var res = new SignalObserver::DataReadBuffer;

        res->pos = d.pos_;
        res->events = data.wellKnownEvents;
        res->method <<= avgr;
        res->ndata = 1;
        res->uptime = data.uptime;
        size_t offs = d.mb_->rd_ptr() - d.mb_->base();
        size_t len = ( ( d.mb_->length() - offs ) / sizeof(long) ) - 32;
		(void)len;
		unsigned char * pchar = reinterpret_cast<unsigned char *>( d.mb_->rd_ptr() );
		pchar += 32 * sizeof(long);

		assert( len == ( data.nbrSamples * 3 / 4 + 1) );

		res->array.length( data.nbrSamples ); // 24bit signed --> 32bit signed
		for ( size_t i = 0; i < data.nbrSamples; ++i ) {
			res->array[i] = pchar[0] << 16 | pchar[1] << 8 | pchar[2];
			if ( pchar[0] & 0x80 )
				res->array[i] |= 0xff000000;
			pchar += 3;
		}
        dataReadBuffer = res._retn();
        return true;
    }

	return false;
}

::CORBA::WChar *
tofObserver_i::dataInterpreterClsid (void)
{
    return CORBA::wstring_dup( L"tofSpectrometer" );
}

CORBA::Long
tofObserver_i::posFromTime( CORBA::ULongLong usec )
{
    return -1;
}

void
tofObserver_i::push_profile_data( ACE_Message_Block * mb )
{
    TAO_InputCDR cdr( mb->rd_ptr(), sizeof(unsigned long) * 32 );
    
    TOFInstrument::AveragerData data;
    CORBA::ULong clsid;
    cdr >> clsid;
    assert( clsid == TOFConstants::ClassID_ProfileData );
    cdr >> data;

    unsigned long prevEvents = 0;
    do {
        acewrapper::scoped_mutex_t<> lock( mutex_ );
        if ( ! fifo_.empty() )
            prevEvents = fifo_.back().wellKnownEvents_;

        fifo_.push_back( cache_item( data.npos, mb, data.wellKnownEvents ) );

        if ( fifo_.size() > 64 )
            fifo_.pop_front();
    } while(0);

    task_.observer_fire_on_update_data( objId_, data.npos );

    if ( prevEvents != data.wellKnownEvents )
      task_.observer_fire_on_event( objId_, data.wellKnownEvents, data.npos );

    //void observer_fire_on_method_changed( long pos );
}

/////////////////////// cache item ////////////////////////////

tofObserver_i::cache_item::~cache_item()
{
    ACE_Message_Block::release( mb_ );
}

tofObserver_i::cache_item::cache_item( long pos, ACE_Message_Block * mb
                                      , unsigned long event ) : pos_(pos)
                                                              , mb_(mb)
                                                              , wellKnownEvents_(event)
{
}

tofObserver_i::cache_item::cache_item( const cache_item& t ) : pos_(t.pos_), mb_(0), wellKnownEvents_(t.wellKnownEvents_)
{
   if ( t.mb_ )
       mb_ = t.mb_->duplicate();
}

tofObserver_i::cache_item::operator long() const
{
    return pos_;
}

