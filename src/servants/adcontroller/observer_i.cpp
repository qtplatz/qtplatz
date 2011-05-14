/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include <algorithm>
#include <acewrapper/mutex.hpp>

namespace adcontroller {

	namespace internal {

		struct observer_events_data {
			bool operator == ( const observer_events_data& ) const;
			bool operator == ( const SignalObserver::ObserverEvents_ptr ) const;
			SignalObserver::ObserverEvents_var events_;
			std::wstring token_;
			SignalObserver::eUpdateFrequency freq_;
			observer_events_data() {}
			observer_events_data( const observer_events_data& t ) : events_( t.events_ )
				                                                  , token_( t.token_ )
																  , freq_( t.freq_ )  {
			}
        };

		struct sibling_data {
			boost::shared_ptr< observer_i > pCache_i_;
            SignalObserver::Observer_var observer_;  // instrument oberver ( in instrument fifo )
            SignalObserver::Observer_var cache_;     // cache observer (in server cache) := pCache_i_
            unsigned long objId_;
			sibling_data() : objId_(0) {}
			sibling_data( const sibling_data& t ) : objId_( t.objId_ )
				                                  , pCache_i_( t.pCache_i_ )
                                                  , observer_( t.observer_ ) 
                                                  , cache_( t.cache_ ) {
            }
        };
      
	}
}

using namespace adcontroller;

observer_i::observer_i( SignalObserver::Observer_ptr source ) : objId_(0)
{
	source_observer_ = SignalObserver::Observer::_duplicate(source);
    if ( ! CORBA::is_nil( source_observer_.in() ) ) {
        cache_.reset( new Cache() );
    }
}

observer_i::~observer_i(void)
{
}

::SignalObserver::Description * 
observer_i::getDescription (void)
{
    return source_observer_->getDescription();
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
						, const CORBA::WChar * token )
{
	using namespace adcontroller::internal;

    observer_events_data data;
    data.events_ = cb;
    data.token_ = token;
    data.freq_ = frequency;

    acewrapper::scoped_mutex_t<> lock( mutex_ );

	observer_events_set_.push_back( data );
	return true;
}

::CORBA::Boolean
observer_i::disconnect ( ::SignalObserver::ObserverEvents_ptr cb )
{
    acewrapper::scoped_mutex_t<> lock( mutex_ );
    
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

    acewrapper::scoped_mutex_t<> lock( mutex_ );

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

    acewrapper::scoped_mutex_t<> lock( mutex_ );

	if ( ! CORBA::is_nil( data.observer_ ) ) {

		data.objId_ = data.observer_->objId();
		data.pCache_i_.reset( new observer_i( data.observer_ ) );  // shadow (cache) observer
		if ( data.pCache_i_ ) {
			data.pCache_i_->assign_objId( data.objId_ );
			PortableServer::POA_var poa = adcontroller::singleton::manager::instance()->poa();
			CORBA::Object_ptr obj = poa->servant_to_reference( data.pCache_i_.get() );
			data.cache_ = SignalObserver::Observer::_narrow( obj );
		}
		data.pCache_i_->populate_siblings();
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
    acewrapper::scoped_mutex_t<> lock( mutex_ );

    for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it ) {
        if ( it->cache_->objId() == objId )
            return SignalObserver::Observer::_duplicate( it->cache_.in() );
    }

    if ( recursive ) {
        ::SignalObserver::Observer * pres = 0;
        for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it ) {
            if ( pres = it->cache_->findObserver( objId, true ) )
                return pres;
        }
    }
    return 0;
}

void
observer_i::uptime ( ::CORBA::ULongLong_out usec )
{
	if ( ! CORBA::is_nil( source_observer_ ) )
		source_observer_->uptime( usec );
	usec = 0;
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

namespace adcontroller {

	namespace internal {

		struct fire_on_update_data {
			static void fire( SignalObserver::ObserverEvents& oe, unsigned long objId, long pos ) {
				oe.OnUpdateData( objId, pos );
			}
		};

		struct fire_on_method_changed {
			static void fire( SignalObserver::ObserverEvents& oe, unsigned long objId, long pos ) {
				oe.OnMethodChanged( objId, pos );
			}
		};

        struct fire_events {
            unsigned long objId_;
            long pos_;
            unsigned long events_;
            fire_events( unsigned long objId, long pos, unsigned long events ) : objId_(objId), pos_(pos), events_(events) {}
            void operator()( internal::observer_events_data& d ) {
                if ( ! CORBA::is_nil( d.events_.in() ) )
                    d.events_->OnEvent( objId_, pos_, events_ );
            }
        };

        template<class T> struct invoke_event_fire {
            unsigned long objId_;
			long pos_;
            unsigned long event_;
			invoke_event_fire( unsigned long objId, long pos, unsigned long e = 0 ) : objId_(objId), pos_(pos), event_(e) {}
			void operator()( internal::observer_events_data& d ) {
				if ( ! CORBA::is_nil( d.events_.in() ) )
                    T::fire( *d.events_, objId_,  pos_ );
			}
		};

	}
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
        if ( p = it->pCache_i_->find_cache_observer( objId ) )
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

bool
observer_i::handle_data( unsigned long /* parentId */, unsigned long objId, long pos )
{
    observer_i * pCache = find_cache_observer( objId );
    if ( pCache && ! CORBA::is_nil( pCache->source_observer_.in() ) ) {
        SignalObserver::DataReadBuffer_var rdbuf;
        if ( pCache->source_observer_->readData( pos, rdbuf ) ) {
            // TODO: handle wellKnownEvents and save data into datafile if necessary
            pCache->write_cache( pos, rdbuf );
        }
    }
	return false;
}

bool
observer_i::forward_notice_update_data( unsigned long parentId, unsigned long objId, long pos )
{
    // this object mast be 'master observer' instance
    assert( objId_ == 0 );

    for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it ) {

        // send notification from child to parent
        if ( it->objId_ == parentId ) {
#if defined _DEBUG
            assert( ( it->objId_ == objId ) || ( it->pCache_i_->isChild( objId ) ) );
#endif
            using namespace adcontroller::internal;
            std::for_each ( it->pCache_i_->events_begin(), it->pCache_i_->events_end(), invoke_event_fire<fire_on_update_data>( objId, pos ) );
            return true;

        }

    }
	return false;
}

bool
observer_i::forward_notice_method_changed( unsigned long /* parentId */, unsigned long objid, long pos )
{
    for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it ) {

        if ( ( it->objId_ == objid ) || it->pCache_i_->isChild( objid ) ) {
            using namespace adcontroller::internal;
            std::for_each ( it->pCache_i_->events_begin(), it->pCache_i_->events_end(), invoke_event_fire<fire_on_method_changed>( objid, pos ) );
            return true;
        }
    }
	return false;
}

bool
observer_i::forward_notice_update_events( unsigned long /* parentId */, unsigned long objid, long pos, unsigned long events )
{
    for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it ) {

        if ( ( it->objId_ == objid ) || it->pCache_i_->isChild( objid ) ) {
            using namespace adcontroller::internal;
            std::for_each ( it->pCache_i_->events_begin(), it->pCache_i_->events_end(), fire_events( objid, pos, events ) );
            return true;
        }
    }
	return false;
}

bool
observer_i::write_cache( long pos, SignalObserver::DataReadBuffer_var& rdbuf )
{
    return cache_->write( pos, rdbuf );
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


