// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include <adinterface/signalobserverS.h>
#include <adinterface/instrumentC.h>

#include <adportable/configuration.hpp>
#include <boost/noncopyable.hpp>

namespace adcontroller {

    class iTask;
    class iProxy;

    class oProxy : public POA_SignalObserver::ObserverEvents, boost::noncopyable {
    public:
        ~oProxy();
        oProxy( iTask& );

        // POA_SignalObserver::ObserverEvents implementation
        virtual void OnConfigChanged ( ::CORBA::ULong objId, ::SignalObserver::eConfigStatus status );
        virtual void OnUpdateData ( ::CORBA::ULong objId, ::CORBA::Long pos );
        virtual void OnMethodChanged ( ::CORBA::ULong objId, ::CORBA::Long pos );
        virtual void OnEvent ( ::CORBA::ULong objId, ::CORBA::ULong event,	::CORBA::Long pos );

        // oProxy implementation
        bool initialize();
        bool connect( const std::string& token );
        bool disconnect();
        bool setInstrumentSession( Instrument::Session_ptr p );
        size_t populateObservers( unsigned long objId );
        void setConfiguration( const adportable::Configuration& );
        void objId( unsigned long objid );
        unsigned long objId() const;
        SignalObserver::Observer_ptr getObject();

    private:
        bool objref_;
        unsigned long objId_;
        iTask& task_;
        Instrument::Session_var iSession_;
        SignalObserver::Observer_var impl_;
        adportable::Configuration config_;
    };
    
}
