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

#pragma warning(disable:4996)
#include <adinterface/signalobserverS.h>
#include <adinterface/instrumentC.h>
#pragma warning(default:4996)

#include <adportable/configuration.h>
#include <boost/noncopyable.hpp>

namespace adcontroller {

    class iBroker;
    class iProxy;

	class oProxy : public POA_SignalObserver::ObserverEvents, boost::noncopyable {
    public:
		~oProxy();
		oProxy( iBroker& );

		// POA_SignalObserver::ObserverEvents implementation
        virtual void OnUpdateData ( ::CORBA::ULong objId, ::CORBA::Long pos );
        virtual void OnMethodChanged ( ::CORBA::ULong objId, ::CORBA::Long pos );
        virtual void OnEvent ( ::CORBA::ULong objId, ::CORBA::ULong event,	::CORBA::Long pos );

		// oProxy implementation
        bool initialize();
		bool connect( const std::wstring& token );
		bool setInstrumentSession( Instrument::Session_ptr p );
		size_t populateObservers( unsigned long objId );
        void setConfiguration( const adportable::Configuration& );
        void objId( unsigned long objid );
		unsigned long objId() const;
		SignalObserver::Observer_ptr getObject();

    private:
		bool objref_;
		unsigned long objId_;
		iBroker& broker_;
		Instrument::Session_var iSession_;
		SignalObserver::Observer_var impl_;
        adportable::Configuration config_;
    };
    
}
