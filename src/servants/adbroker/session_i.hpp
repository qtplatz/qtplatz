// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#pragma once

# include "adinterface/brokerS.h"
# include "adinterface/brokereventC.h"


#include <vector>

namespace adbroker {

	namespace internal {
        struct event_sink;
	}

    class session_i : public POA_Broker::Session {
        PortableServer::ObjectId oid_;
    public:
        void oid( const PortableServer::ObjectId& oid ) { oid_ = oid; }
        const PortableServer::ObjectId& oid() { return oid_; }

        session_i( const wchar_t * token = 0 );
        ~session_i(void);
        // implement POA_Broker::Session -->
        virtual bool connect( const char * user, const char * pass, const char * token, BrokerEventSink_ptr );
        virtual bool disconnect( BrokerEventSink_ptr );

        virtual Broker::ChemicalFormula_ptr getChemicalFormula();
        virtual bool coaddSpectrum ( SignalObserver::Observer_ptr observer, double x1, double x2);
        virtual Broker::Folium * folium( const CORBA::WChar * token, const CORBA::WChar * fileId );
        // <---------------------------------
		virtual bool coaddSpectrumEx( const CORBA::WChar * token, SignalObserver::Observer_ptr observer, double x1, double x2);
    private:
        std::wstring token_;

        Broker::ChemicalFormula_var chemicalFormula_;

        typedef std::vector<internal::event_sink> event_sink_vector_type;

        event_sink_vector_type event_sink_set_;
        inline event_sink_vector_type::iterator begin() { return event_sink_set_.begin(); };
		inline event_sink_vector_type::iterator end()   { return event_sink_set_.end(); };

    };

}
