/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "tofobserver_i.hpp"
#include "toftask.hpp"
#include <adportable/debug.hpp>

using namespace tofservant;

//////////////////////////////////

tofObserver_i::tofObserver_i( toftask& t ) : task_(t)
                                           , objId_(0)
{
    desc_.trace_method = SignalObserver::eTRACE_SPECTRA;
    desc_.spectrometer = SignalObserver::eMassSpectrometer;
    desc_.trace_id = CORBA::wstring_dup( L"MS.PROFILE" );
    desc_.trace_display_name = CORBA::wstring_dup( L"InfiTOF MS Spectrum" );
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
    adportable::scope_timer<>( "tofObserver_i::assignObjeId: ");
    objId_ = oid;
    std::cerr << "tofObserver_i::assign_objId(" << oid << ")" << std::endl;
}

::CORBA::Boolean
tofObserver_i::connect ( ::SignalObserver::ObserverEvents_ptr cb
                         , ::SignalObserver::eUpdateFrequency frequency
                         , const CORBA::Char * token )
{
    adportable::scope_timer<>( "tofObserver_i::connect: ");
    Logger( L"tofObserver_i::connect from %1% on id=%2%(%3%)" ) % token % objId_ % this;
    return task_.connect( cb, frequency, token );
}

::CORBA::Boolean
tofObserver_i::disconnect( ::SignalObserver::ObserverEvents_ptr cb )
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
    adportable::scope_timer<>( "tofObserver_i::getSiblings: ");

    SignalObserver::Observers_var vec( new SignalObserver::Observers );
    vec->length( siblings_.size() );

    for ( size_t i = 0; i < siblings_.size(); ++i )
        (*vec)[i] = SignalObserver::Observer::_duplicate( siblings_[i].in() );

    return vec._retn();
}

::SignalObserver::Observer *
tofObserver_i::findObserver( CORBA::ULong objId, CORBA::Boolean recursive )
{
    // adportable::scope_timer<>( "tofObserver_i::findObserver: ");

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
tofObserver_i::addSibling ( ::SignalObserver::Observer_ptr observer )
{
    adportable::scope_timer<>( "tofObserver_i::addSibling: ");

    ::SignalObserver::Observer_var var = SignalObserver::Observer::_duplicate( observer );
    siblings_.push_back( var );
    return true;
}

void
tofObserver_i::uptime ( ::CORBA::ULongLong_out usec )
{
    ACE_UNUSED_ARG( usec );
}

void
tofObserver_i::uptime_range( ::CORBA::ULongLong_out oldest, ::CORBA::ULongLong_out newest )
{
    ACE_UNUSED_ARG( oldest );
    ACE_UNUSED_ARG( newest );
}

::CORBA::Boolean
tofObserver_i::readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer )
{
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

        if ( d.mb_->msg_type() == constants::MB_InfiTOF_DATA ) {

            TAO_InputCDR cdr( d.mb_->cont() );
            InfiTOF::AveragerData data;
            cdr >> data;

            SignalObserver::DataReadBuffer_var res = new SignalObserver::DataReadBuffer;

            res->method <<= data;
            res->uptime = data.uptime;
            res->pos = d.pos_;
            res->events = data.wellKnownEvents;

            res->array.length( data.nbrSamples );
            memcpy( &res->array[0], d.mb_->rd_ptr(), data.nbrSamples * sizeof(long) );

            dataReadBuffer = res._retn();
            return true;
        }
    }
    return false;
}

::CORBA::WChar *
tofObserver_i::dataInterpreterClsid (void)
{
    return CORBA::wstring_dup( constants::dataInterpreter::spectrometer::name() ); // L"InfiTOF"
}

CORBA::Long
tofObserver_i::posFromTime( CORBA::ULongLong usec )
{
    (void)usec;
    return (-1);
}

void
tofObserver_i::debug( const InfiTOF::AveragerData& avgr, const Acqrs::AqDescriptors& desc )
{
    const Acqrs::AqSegmentDescriptorAvg& segDesc = desc.segDesc;
    const Acqrs::AqDataDescriptor& ddesc = desc.dataDesc;
    (void)ddesc;
    (void)avgr;
    
    unsigned long flags = segDesc.flags;  // 0:P1, 1:P2, 2:A, 3:B
    std::string name = "[";
    if ( flags & 1 )
        name += "P1,";
    if ( flags & 2 )
        name += "P2,";
    if ( flags & 4 )
        name += "A,";
    if ( flags & 8 )
        name += "B,";
    name += "]";

    static unsigned long long last;
    unsigned long long timestamp = CORBA::ULongLong( segDesc.timeStampHi ) << 32 | segDesc.timeStampLo;
#if defined DEBUG && defined VERBOSE
    std::cerr << "AcqrsD1_readData[" << avgr.npos << "]"
              << std::fixed << std::setprecision(1) << double( timestamp ) / 1.0e9 // ps --> ms
              << "\t" << double( timestamp - last ) / 1.0e9 // ps --> ms
              << "\t0x" << std::hex << flags << name
              << "\tsampTime: " << std::scientific << ddesc.sampTime
              << "\t" << std::dec << ddesc.returnedSamplesPerSeg
              << "\t" << ddesc.nbrAvgWforms
              << std::endl;
#endif    
    last = timestamp;
    (void)last;
}

void
tofObserver_i::push_profile_data( ACE_Message_Block * mblk, long npos, unsigned long wellKnownEvents )
{
    if ( mblk->msg_type() != constants::MB_InfiTOF_DATA ) {
        adportable::debug() << "tofObserver_i::push_profile_data -- not MB_InfiTOF_DATA";
        return;
    }
    unsigned long prevEvents = 0;

    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( ! fifo_.empty() )
            prevEvents = fifo_.back().wellKnownEvents_;
        fifo_.push_back( cache_item( npos, mblk, wellKnownEvents ) );
        
        if ( fifo_.size() > 64 )
            fifo_.pop_front();
    } while(0);
    
    task_.observer_fire_on_update_data( objId_, npos );

    if ( prevEvents != wellKnownEvents )
        task_.observer_fire_on_event( objId_, wellKnownEvents, npos );
}

/////////////////////// cache item ////////////////////////////

tofObserver_i::cache_item::~cache_item()
{
    ACE_Message_Block::release( mb_ );
    
}

tofObserver_i::cache_item::cache_item( long pos, ACE_Message_Block * mb
                                      , unsigned long event ) : pos_(pos)
                                                              , wellKnownEvents_(event)
                                                              , mb_( ACE_Message_Block::duplicate( mb ) )
{
}

tofObserver_i::cache_item::cache_item( const cache_item& t ) : pos_(t.pos_)
                                                             , wellKnownEvents_(t.wellKnownEvents_)
                                                             , mb_( ACE_Message_Block::duplicate( t.mb_ ) )
{
}

tofObserver_i::cache_item::operator long() const
{
    return pos_;
}

