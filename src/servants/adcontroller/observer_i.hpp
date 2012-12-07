// -*- C++ -*-
/**************************************************************************
 ** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
 ** Science Liaison / Advanced Instrumentation Project
 *
 ** Contact: toshi.hondo@scienceliaison.com
 **
 ** Commercial Usage
 **
 ** Licensees holding valid ScienceLiaison commercial licenses may use this
 ** file in accordance with the ScienceLiaison Commercial License Agreement
 ** provided with the Software or, alternatively, in accordance with the terms
 ** contained in a written agreement between you and ScienceLiaison.
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

#pragma once

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>

# include <adinterface/signalobserverS.h>
# include <ace/Recursive_Thread_Mutex.h>

namespace adcontroller {

    namespace internal {
        struct observer_events_data;
        struct sibling_data;
    }

    class Cache;

    class observer_i : public virtual POA_SignalObserver::Observer, boost::noncopyable {
    public:
        observer_i( SignalObserver::Observer_ptr source = 0 );
        ~observer_i(void);

        virtual ::SignalObserver::Description * getDescription (void);
        virtual ::CORBA::Boolean setDescription ( const ::SignalObserver::Description & desc );
        virtual ::CORBA::ULong objId();
        virtual void assign_objId( CORBA::ULong oid );

        virtual ::CORBA::Boolean connect( ::SignalObserver::ObserverEvents_ptr cb
                                          , ::SignalObserver::eUpdateFrequency frequency
                                          , const CORBA::Char * );
        virtual ::CORBA::Boolean disconnect( ::SignalObserver::ObserverEvents_ptr cb );
        virtual ::CORBA::Boolean isActive (void);
        virtual ::SignalObserver::Observers * getSiblings (void);
        virtual ::CORBA::Boolean addSibling ( ::SignalObserver::Observer_ptr observer);
        virtual ::SignalObserver::Observer * findObserver( CORBA::ULong objId, CORBA::Boolean recursive );
        virtual void uptime ( ::CORBA::ULongLong_out usec );
        virtual void uptime_range( ::CORBA::ULongLong_out oldest, ::CORBA::ULongLong_out newest );
        virtual ::CORBA::Boolean readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer);
        virtual ::CORBA::WChar * dataInterpreterClsid (void);
        virtual ::CORBA::Long posFromTime( ::CORBA::ULongLong usec );
        /// <-- end implementation ---
        //--------------------------------------------
        void populate_siblings();
        bool isChild( unsigned long objid );
        bool handle_data( unsigned long parentId, unsigned long objId, long pos );
        bool forward_observer_update_data( unsigned long parentId, unsigned long objid, long pos );
        bool forward_observer_update_method( unsigned long parentId, unsigned long objid, long pos );
        bool forward_observer_update_events( unsigned long parentId, unsigned long objid, long pos, unsigned long events );

    private:
        typedef std::vector<internal::observer_events_data> observer_events_vector_type;
        typedef std::vector<internal::sibling_data> sibling_vector_type;
      
        inline observer_events_vector_type::iterator events_begin() { return observer_events_set_.begin(); };
        inline observer_events_vector_type::iterator events_end()   { return observer_events_set_.end(); };

        inline sibling_vector_type::iterator sibling_begin() { return sibling_set_.begin(); };
        inline sibling_vector_type::iterator sibling_end()   { return sibling_set_.end(); };

        observer_i * find_cache_observer( unsigned long );
        bool write_cache( long pos, SignalObserver::DataReadBuffer_var& );

        observer_events_vector_type observer_events_set_;
        sibling_vector_type sibling_set_;
        SignalObserver::Observer_var source_observer_;
        SignalObserver::Description desc_;
        unsigned long objId_;
        boost::scoped_ptr< Cache > cache_;
        ACE_Recursive_Thread_Mutex mutex_;
    };

}
