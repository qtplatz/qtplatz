// This is a -*- C++ -*- header.
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

#pragma once

#include "tofinterface/tofS.h"
#include <adinterface/controlmethodC.h>

#include <vector>

namespace tofservant {

    class tofSession_i : public POA_TOF::Session {
    public:
        char * software_revision();  // ex. L"1.216"

        // setConfiguration will call immedate after object activated.
        bool setConfiguration( const char * xml ) override;

        bool configComplete() override;
    
        bool connect( Receiver_ptr receiver, const char * token) override;
        bool disconnect( Receiver_ptr receiver_) override;
      
		CORBA::ULong get_status() override;
        SignalObserver::Observer * getObserver() override;
      
        bool initialize() override;
        bool shutdown() override;  // shutdown server
        bool echo( const char * msg ) override;
        bool shell( const char * cmdline ) override;
		::ControlMethod::Method * getControlMethod() override;
        bool prepare_for_run( const ControlMethod::Method& ) override;
        bool push_back( SampleBroker::SampleSequence_ptr ) override;
    
        bool event_out( CORBA::ULong event) override;
        bool start_run() override;
        bool suspend_run() override;
        bool resume_run() override;
        bool stop_run() override;

        //
        void debug( const CORBA::WChar * text, const CORBA::WChar * key );
		bool setControlMethod( const TOF::ControlMethod& tof, const CORBA::Char * hint );
    };

}
