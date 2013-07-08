// This is a -*- C++ -*- header.
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

#pragma once

# include <tofinterface/tofS.h>
# include <adinterface/signalobserverS.h>

#include <vector>
#include <deque>
#include <mutex>

namespace TOFSignal { struct tofDATA; }

namespace tofservant {

    class profileObserver_i : public virtual POA_SignalObserver::Observer {
    public:
        profileObserver_i();
        ~profileObserver_i(void);

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
        virtual ::CORBA::Long posFromTime( CORBA::ULongLong usec );

        // internal
        void push_profile_data( std::shared_ptr< TOFSignal::tofDATA>&, long npos, unsigned long wellKnownEvents );

    private:
        std::mutex mutex_;        

        unsigned long objId_;
        SignalObserver::Description desc_;
        typedef std::vector< ::SignalObserver::Observer_var > sibling_vector_type;
        sibling_vector_type siblings_;

        struct cache_item {
            ~cache_item();
            cache_item( long pos, std::shared_ptr< TOFSignal::tofDATA >&, unsigned long event );
            cache_item( const cache_item & );
            operator long () const;
            long pos_;
            unsigned long wellKnownEvents_;
            std::shared_ptr< TOFSignal::tofDATA > data_;
        };
        std::deque< cache_item > fifo_;
    };

}
