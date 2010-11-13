// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#pragma warning (disable : 4996 )
# include "tofcontrollerS.h"
# include <adinterface/signalobserverS.h>
#pragma warning (default : 4996 )

#include <vector>
#include <deque>

namespace tofcontroller {

    class Task;

	class tofObserver_i : public virtual POA_SignalObserver::Observer {
	public:
		tofObserver_i( Task& );
		~tofObserver_i(void);

		virtual ::SignalObserver::Description * getDescription (void);
		virtual ::CORBA::Boolean setDescription ( const ::SignalObserver::Description & desc );
		virtual ::CORBA::ULong objId();
		virtual void assign_objId( CORBA::ULong oid );
		virtual ::CORBA::Boolean connect( ::SignalObserver::ObserverEvents_ptr cb
			                             , ::SignalObserver::eUpdateFrequency frequency
										 , const CORBA::WChar * );
        virtual ::CORBA::Boolean disconnect( ::SignalObserver::ObserverEvents_ptr cb );
		virtual ::CORBA::Boolean isActive (void);
		virtual ::SignalObserver::Observers * getSiblings (void);
		virtual ::CORBA::Boolean addSibling ( ::SignalObserver::Observer_ptr observer);
        virtual ::SignalObserver::Observer * findObserver( CORBA::ULong objId, CORBA::Boolean recursive );
		virtual void uptime ( ::CORBA::ULongLong_out usec );
		virtual ::CORBA::Boolean readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer);
		virtual ::CORBA::WChar * dataInterpreterClsid (void);

        // internal
        void push_profile_data( ACE_Message_Block * mb );

	private:

        ACE_Recursive_Thread_Mutex mutex_;        
		Task & task_;
        unsigned long objId_;
		SignalObserver::Description desc_;
        typedef std::vector< ::SignalObserver::Observer_var > sibling_vector_type;
        sibling_vector_type siblings_;

        struct cache_item {
            ~cache_item();
            cache_item( long pos, ACE_Message_Block * mb, unsigned long event );
            cache_item( const cache_item & );
            operator long () const;
            long pos_;
            unsigned long wellKnownEvents_;
            ACE_Message_Block * mb_;
        };
        std::deque< cache_item > fifo_;
	};

}
