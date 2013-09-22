/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "observer_i.hpp"
#include "manager_i.hpp"
#include "cache.hpp"
#include "task.hpp"
#include "sampleprocessor.hpp"
#include <algorithm>
#include <acewrapper/orbservant.hpp>
#include <adportable/debug.hpp>
#include <adportable/timer.hpp>
#include "logging.hpp"

namespace adcontroller {

    namespace internal {
	
        struct observer_events_data {
            bool operator == ( const observer_events_data& ) const;
            bool operator == ( const SignalObserver::ObserverEvents_ptr ) const;
            SignalObserver::ObserverEvents_var events_;
            std::string token_;
            SignalObserver::eUpdateFrequency freq_;
            observer_events_data() {}
            observer_events_data( const observer_events_data& t ) : events_( t.events_ )
                                                                  , token_( t.token_ )
                                                                  , freq_( t.freq_ )  {
            }
        };
	
        struct sibling_data {
            std::shared_ptr< observer_i > pCache_i_;
            SignalObserver::Observer_var observer_;  // instrument oberver ( in instrument fifo )
            SignalObserver::Observer_var cache_;     // cache observer (in server cache) := pCache_i_
            unsigned long objId_;
            sibling_data() : objId_(0) {}
            sibling_data( const sibling_data& t ) : pCache_i_( t.pCache_i_ )
                                                  , observer_( t.observer_ ) 
                                                  , cache_( t.cache_ )
                                                  , objId_( t.objId_ ) {
            }
        };
	
    }
}

using namespace adcontroller;

observer_i::observer_i( SignalObserver::Observer_ptr source ) : objId_(0)
                                                              , npos_i_have_(0)
{
    source_observer_ = SignalObserver::Observer::_duplicate(source);
	if ( ! CORBA::is_nil( source_observer_.in() ) )
		cache_.reset( new Cache() );
}

observer_i::~observer_i(void)
{
}

::SignalObserver::Description * 
observer_i::getDescription (void)
{
    try {
        return source_observer_->getDescription();
    } catch ( CORBA::Exception& ex ) {
        adportable::debug( __FILE__, __LINE__ ) << ex._info().c_str();
    }
    return 0;
}

::CORBA::Boolean
observer_i::setDescription ( const ::SignalObserver::Description & desc )
{
    desc_ = desc;
    source_observer_->setDescription( desc );
    return true;
}

CORBA::ULong
observer_i::objId()
{
    return objId_;
}

void
observer_i::assign_objId( CORBA::ULong oid )
{
    objId_ = oid;
}

::CORBA::Boolean
observer_i::connect ( ::SignalObserver::ObserverEvents_ptr cb
                      , ::SignalObserver::eUpdateFrequency frequency
                      , const CORBA::Char * token )
{
    using namespace adcontroller::internal;

    observer_events_data data;
    data.events_ = cb;
    data.token_ = token;
    data.freq_ = frequency;

    Logging( L"observer_i::connect from %1% frequency: %2% token: %3%" ) % (void *)cb % frequency % token;
    
    std::lock_guard< std::mutex > lock( mutex_ );
    
    observer_events_set_.push_back( data );
    return true;
}

::CORBA::Boolean
observer_i::disconnect ( ::SignalObserver::ObserverEvents_ptr cb )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    
    observer_events_vector_type::iterator it
        = std::find(observer_events_set_.begin(), observer_events_set_.end(), cb);

    if ( it != observer_events_set_.end() ) {
        observer_events_set_.erase( it );
        return true;
    }
    return false;
}

::CORBA::Boolean
observer_i::isActive (void)
{
    return true;
}

::SignalObserver::Observers *
observer_i::getSiblings (void)
{
    SignalObserver::Observers_var vec( new SignalObserver::Observers );
    vec->length( sibling_set_.size() );

    std::lock_guard< std::mutex > lock( mutex_ );

    int i = 0;
    for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it )
        (*vec)[i++] = SignalObserver::Observer::_duplicate( it->cache_.in() );

    return vec._retn();
}

::CORBA::Boolean
observer_i::addSibling ( ::SignalObserver::Observer_ptr observer )
{
    internal::sibling_data data;
    data.observer_ = SignalObserver::Observer::_duplicate( observer ); // real observer points to instrumets

    std::lock_guard< std::mutex > lock( mutex_ );

    if ( ! CORBA::is_nil( data.observer_ ) ) {

		adportable::timer x;

		data.objId_ = data.observer_->objId();

		adportable::timer y;

        data.pCache_i_.reset( new observer_i( data.observer_ ) );  // shadow (cache) observer

        if ( data.pCache_i_ ) {
            data.pCache_i_->assign_objId( data.objId_ );
            PortableServer::POA_var poa = adcontroller::manager_i::instance()->poa();
            CORBA::Object_ptr obj = poa->servant_to_reference( data.pCache_i_.get() );
            data.cache_ = SignalObserver::Observer::_narrow( obj );
        }
		data.pCache_i_->populate_siblings();
		SignalObserver::Description_var desc = data.pCache_i_->getDescription();
		Logging(L"observer_i::addSibling(%1%) (elapsed = %2%,%3%)", ::EventLog::pri_INFO )
			% desc->trace_display_name.in() % x.elapsed() % y.elapsed();
    }
    sibling_set_.push_back( data );
    return true;
}

void
observer_i::populate_siblings()
{
	if ( CORBA::is_nil( source_observer_ ) )
		return;

	SignalObserver::Observers_var sourceVec = source_observer_->getSiblings();
	if ( sourceVec.ptr() == 0 )
		return;

	size_t nsize = sourceVec->length();

	for ( size_t i = 0; i < nsize; ++i )
        addSibling( sourceVec[i] );
}

::SignalObserver::Observer *
observer_i::findObserver( CORBA::ULong objId, CORBA::Boolean recursive )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it ) {
        if ( it->cache_->objId() == objId )
            return SignalObserver::Observer::_duplicate( it->cache_.in() );
    }

    if ( recursive ) {
        ::SignalObserver::Observer * pres = 0;
        for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it ) {
            if ( ( pres = it->cache_->findObserver( objId, true ) ) )
                return pres;
        }
    }
    return 0;
}

void
observer_i::uptime ( ::CORBA::ULongLong_out usec )
{
    usec = 0;
    unsigned long long newest;
    unsigned long long oldest;
    if ( cache_ ) {
        cache_->uptime_range( oldest, newest );
        usec = newest;
    }
}

void
observer_i::uptime_range( ::CORBA::ULongLong_out oldest, ::CORBA::ULongLong_out newest )
{
    oldest = 0;
    newest = 0;
    if ( cache_ ) {
        unsigned long long t0, t1;
        cache_->uptime_range( t0, t1 );
        oldest = t0;
        newest = t1;
    }
}

::CORBA::Boolean
observer_i::readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer )
{
    return cache_->read( pos, dataReadBuffer );
}

::CORBA::WChar *
observer_i::dataInterpreterClsid (void)
{
	if ( ! CORBA::is_nil( source_observer_ ) )
		return source_observer_->dataInterpreterClsid();
	return 0;
}

CORBA::Long
observer_i::posFromTime( CORBA::ULongLong usec )
{
    return cache_->posFromTime( usec );    
}

CORBA::Boolean
observer_i::readCalibration( CORBA::ULong idx, SignalObserver::octet_array_out data, CORBA::WString_out dataClass )
{
	if ( ! CORBA::is_nil( source_observer_ ) )
		return source_observer_->readCalibration( idx, data, dataClass );
	return false;
}

observer_i *
observer_i::find_cache_observer( unsigned long objId )
{
    for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it ) {
        if ( it->objId_ == objId )
            return it->pCache_i_.get();
    }

    observer_i * p(0);
    for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it )
        if ( ( p = it->pCache_i_->find_cache_observer( objId ) ) )
            return p;
    return 0;
}


bool
observer_i::isChild( unsigned long objid )
{
    for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it ) {
        if ( it->objId_ == objid )
            return true;
    }

    for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it )
        if ( it->pCache_i_->isChild( objid ) )
            return true;

    return false;
}

SignalObserver::DataReadBuffer *
observer_i::handle_data( unsigned long /* parentId */, unsigned long objId, long pos )
{
    observer_i * pCache = find_cache_observer( objId );
    if ( pCache && ! CORBA::is_nil( pCache->source_observer_.in() ) ) {

        if ( pCache->ihave( pos ) )
            return 0;

        SignalObserver::DataReadBuffer_var rdbuf;
        try {
            if ( pCache->source_observer_->readData( pos, rdbuf ) ) {
                pCache->write_cache( pos, rdbuf ); // copy in to cache (for acquie/tune GuI)
                return rdbuf._retn();
            }
        } catch ( CORBA::Exception& ex ) {
            adportable::debug(__FILE__, __LINE__)
                << "CORBA::Exception: " << ex._info().c_str();
            Logging( L"Exception: observer_i::handle_data \"%1%\"\t while readData from %2% at pos %3%" )
                % ex._info().c_str() % objId % pos;
        }
    }
    return 0;
}

bool
observer_i::forward_observer_update_data( unsigned long parentId, unsigned long objId, long pos )
{
    // this object mast be 'master observer' instance
    assert( objId_ == 0 );

    // fire all event-clients connecting to 'master observer'
    for ( internal::observer_events_data& o: observer_events_set_ ) {
        // todo: check frequency
        o.events_->OnUpdateData( objId, pos );
    }

    // following event firing is optional
    for ( internal::sibling_data& sibling: sibling_set_ ) {
        // send notification from child to parent
        if ( sibling.objId_ == parentId ) {
            using namespace adcontroller::internal;
            for ( observer_events_data& o: sibling.pCache_i_->observer_events_set_ ) {
                // todo: check frequency
                o.events_->OnUpdateData( objId, pos );
            }
        }
    }
    return true;
}

bool
observer_i::forward_observer_update_method( unsigned long /* parentId */, unsigned long objId, long pos )
{
    // this object mast be 'master observer' instance
    assert( objId_ == 0 );

    // fire all event-clients connecting to 'master observer'
    for ( internal::observer_events_data& o: observer_events_set_ )
        o.events_->OnMethodChanged( objId, pos );

    for ( internal::sibling_data& sibling: sibling_set_ ) {
        if ( ( sibling.objId_ == objId ) || sibling.pCache_i_->isChild( objId ) ) {
            using namespace adcontroller::internal;
            for ( observer_events_data& o: sibling.pCache_i_->observer_events_set_ )
                o.events_->OnMethodChanged( objId, pos );
        }
    }
    return true;
}

bool
observer_i::forward_observer_update_events( unsigned long /* parentId */
                                            , unsigned long objId, long pos, unsigned long events )
{
    // fire all event-clients connecting to 'master observer'
    for ( internal::observer_events_data& o: observer_events_set_ )
        o.events_->OnEvent( objId, pos, events );

    // optional
    for ( internal::sibling_data& sibling: sibling_set_ ) {
        if ( ( sibling.objId_ == objId ) || sibling.pCache_i_->isChild( objId ) ) {
            using namespace adcontroller::internal;
            for ( observer_events_data& o: sibling.pCache_i_->observer_events_set_ )
                o.events_->OnEvent( objId, pos, events );
            return true;
        }
    }
    return true;
}

// void
// observer_i::push_sample_processor( std::shared_ptr< SampleProcessor >& ptr )
// {
//     std::lock_guard< std::mutex > lock( mutex_ );
//     queue_.push_back( ptr );
// }

// void
// observer_i::stop_sample_processor()
// {
//     std::lock_guard< std::mutex > lock( mutex_ );
// 	if ( ! queue_.empty() )
// 		queue_.pop_front();
// }

bool
observer_i::write_cache( long pos, SignalObserver::DataReadBuffer_var& rdbuf )
{
    if ( rdbuf->ndata >= 1 ) // spectrum may set 0 to ndata
        npos_i_have_ = pos + rdbuf->ndata - 1;
    return cache_->write( pos, rdbuf );
}

bool
observer_i::ihave( long pos )
{
    return ( unsigned( pos ) <= npos_i_have_ ) ? true : false;
}

///////////////////////////////////
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

/////////////////////////////////////////////


