//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "observer_i.h"
#include "manager_i.h"
#include <algorithm>

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
			SignalObserver::Observer_var observer_;  // real instrument hooked up oberver
			SignalObserver::Observer_var cache_;     // cache observer
            unsigned long objId_;
			sibling_data() : objId_(0) {}
			sibling_data( const sibling_data& t ) : objId_( t.objId_ )
				                                  , observer_( t.observer_ ) 
				                                  , pCache_i_( t.pCache_i_ )
												  , cache_( t.cache_ ) {
			}
        };
      
	}
}

using namespace adcontroller;

observer_i::observer_i( SignalObserver::Observer_ptr source ) : objId_(0)
{
	source_observer_ = SignalObserver::Observer::_duplicate(source);
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
	observer_events_set_.push_back( data );
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
	SignalObserver::Observers_var vec( new SignalObserver::Observers );
    vec->length( sibling_set_.size() );

    int i = 0;
	for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it )
		(*vec)[i++] = SignalObserver::Observer::_duplicate( it->cache_.in() );

	return vec._retn();
}

::CORBA::Boolean
observer_i::addSibling ( ::SignalObserver::Observer_ptr observer )
{
	internal::sibling_data data;
    data.observer_ = SignalObserver::Observer::_duplicate( observer );

	if ( ! CORBA::is_nil( data.observer_ ) ) {

		data.objId_ = data.observer_->objId();
		data.pCache_i_.reset( new observer_i( data.observer_ ) );
		if ( data.pCache_i_ ) {
			data.pCache_i_->assign_objId( data.objId_ );
			PortableServer::POA_var poa = adcontroller::singleton::manager::instance()->getServantManager()->root_poa();
			CORBA::Object_ptr obj = poa->servant_to_reference( data.pCache_i_.get() );
			data.cache_ = SignalObserver::Observer::_narrow( obj );
		}
		data.pCache_i_->populate_siblings();
	}
	sibling_set_.push_back( data );
	return true;
}

::SignalObserver::Observer *
observer_i::findObserver( CORBA::ULong objId, CORBA::Boolean recursive )
{
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
observer_i::populate_siblings()
{
	if ( CORBA::is_nil( source_observer_ ) )
		return;
	SignalObserver::Observers_var sourceVec = source_observer_->getSiblings();
	if ( sourceVec.ptr() == 0 )
		return;
	size_t nsize = sourceVec->length();
	for ( size_t i = 0; i < nsize; ++i ) {
#if defined _DEBUG && 0
        unsigned long oid = sourceVec[i]->objId();
		SignalObserver::Observers_var vec = sourceVec[i]->getSiblings();
		if ( vec.ptr() )
			size_t nn = vec->length();
#endif
		addSibling( sourceVec[i] );
	}
}

void
observer_i::uptime ( ::CORBA::ULongLong_out usec )
{
	if ( ! CORBA::is_nil( source_observer_ ) )
		source_observer_->uptime( usec );
	usec = 0;
}

::CORBA::Boolean
observer_i::readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer)
{
	if ( ! CORBA::is_nil( source_observer_ ) )
		return source_observer_->readData( pos, dataReadBuffer );
	return false;
}

::CORBA::WChar *
observer_i::dataInterpreterClsid (void)
{
	if ( ! CORBA::is_nil( source_observer_ ) )
		return source_observer_->dataInterpreterClsid();
	return 0;
}

namespace adcontroller {
	namespace internal {

		struct fire_on_update_data {
			static void fire( SignalObserver::ObserverEvents& oe, long pos ) {
				oe.OnUpdateData( pos );
			}
		};
		struct fire_on_method_changed {
			static void fire( SignalObserver::ObserverEvents& oe, long pos ) {
				oe.OnMethodChanged( pos );
			}
		};
		struct fire_on_event {
			static void fire( SignalObserver::ObserverEvents& oe, long pos, unsigned long e ) {
				oe.OnEvent( pos, e );
			}
		};

		template<class T> struct invoke_event_fire {
			long pos_;
            unsigned long event_;
			invoke_event_fire( long pos, unsigned long e = 0 ) : pos_(pos), event_(e) {}
			void operator()( internal::observer_events_data& d ) {
				if ( ! CORBA::is_nil( d.events_.in() ) )
					T::fire( *d.events_, pos_ );
			}
		};

	}
}

bool
observer_i::invoke_update_data( unsigned long objid, long pos )
{
	if ( objId_ == objid ) {
		using namespace adcontroller::internal;
		std::for_each ( events_begin(), events_end(), invoke_event_fire<fire_on_update_data>( pos ) );
		return true;
	}

	for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it ) {
		if ( it->pCache_i_->invoke_update_data( objid, pos ) )
			return true;
	}

	return false;
}

bool
observer_i::invoke_method_changed( unsigned long objid, long pos )
{
	if ( objId_ == objid ) {
		using namespace adcontroller::internal;
		std::for_each ( events_begin(), events_end(), invoke_event_fire<fire_on_method_changed>( pos ) );
		return true;
	}

	for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it ) {
		if ( it->pCache_i_->invoke_method_changed( objid, pos ) )
			return true;
	}
	return false;
}

bool
observer_i::invoke_update_event( unsigned long objid, long pos, unsigned long event )
{
	struct fire_event {
		long pos_;
        unsigned long event_;
		fire_event( long pos, unsigned long event ) : pos_(pos), event_(event) {}
		void operator()( internal::observer_events_data& d ) {
			if ( ! CORBA::is_nil( d.events_.in() ) )
				d.events_->OnEvent( pos_, event_ );
		}
	};

	if ( objId_ == objid ) {
		std::for_each ( events_begin(), events_end(), fire_event( pos, event ) );
		return true;
	}

	for ( sibling_vector_type::iterator it = sibling_begin(); it != sibling_end(); ++it ) {
		if ( it->pCache_i_->invoke_update_event( objid, pos, event ) )
			return true;
	}
	return false;
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
