// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#pragma warning (disable : 4996 )
# include <adinterface/signalobserverS.h>
#pragma warning (default : 4996 )

#include <vector>
#include <boost/noncopyable.hpp>

namespace adcontroller {

	namespace internal {
		struct observer_events_data;
        struct sibling_data;
	}

	class observer_i : public virtual POA_SignalObserver::Observer, boost::noncopyable {
	public:
		observer_i();
		~observer_i(void);

		virtual ::SignalObserver::Description * getDescription (void);
		virtual ::CORBA::Boolean setDescription ( const ::SignalObserver::Description & desc );
		virtual ::CORBA::ULong objId();
		virtual void assign_objId( CORBA::ULong oid );

		virtual ::CORBA::Boolean connect( ::SignalObserver::ObserverEvents_ptr cb
			                             , ::SignalObserver::eUpdateFrequency frequency
										 , const CORBA::WChar * );
		virtual ::CORBA::Boolean isActive (void);
		virtual ::SignalObserver::Observers * getSiblings (void);
		virtual ::CORBA::Boolean addSibling ( ::SignalObserver::Observer_ptr observer);
		virtual void uptime ( ::CORBA::ULongLong_out usec );
		virtual ::CORBA::Boolean readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer);
		virtual ::CORBA::WChar * dataInterpreterClsid (void);
		///
		void invoke_update_data( unsigned long objid, long pos );
		void invoke_update_method( unsigned long objid, long pos );
		void invoke_update_event( unsigned long objid, long pos, unsigned long event );

	private:
		typedef std::vector<internal::observer_events_data> observer_events_vector_type;
		typedef std::vector<internal::sibling_data> sibling_vector_type;
      
		inline observer_events_vector_type::iterator events_begin() { return observer_events_set_.begin(); };
		inline observer_events_vector_type::iterator events_end()   { return observer_events_set_.end(); };

		inline sibling_vector_type::iterator sibling_begin() { return sibling_set_.begin(); };
        inline sibling_vector_type::iterator sibling_end()   { return sibling_set_.end(); };

		observer_events_vector_type observer_events_set_;
		sibling_vector_type sibling_set_;
		SignalObserver::Observer_var source_observer_;
		SignalObserver::Description desc_;
        unsigned long objId_;
	};

}
