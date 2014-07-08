// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include <compiler/diagnostic_push.h>
#include <compiler/disable_deprecated.h>

#include <adinterface/receiverC.h>
#include <adinterface/controlserverS.h>
#include <compiler/diagnostic_pop.h>

#include <vector>

namespace adcontroller {

    class session_i : public virtual POA_ControlServer::Session {
    public:
        ~session_i();
        session_i();
      
        ::CORBA::Char * software_revision() override;
        ::CORBA::Boolean connect( Receiver_ptr receiver, const CORBA::Char * token );
        ::CORBA::Boolean disconnect( Receiver_ptr receiver );
        ::CORBA::Boolean setConfiguration( const CORBA::Char * xml );
        ::CORBA::Boolean configComplete();
        ::ControlServer::eStatus status();
        ::CORBA::Boolean initialize();
        ::CORBA::Boolean shutdown();
        ::CORBA::Boolean echo( const CORBA::Char * msg );
        ::CORBA::Boolean shell( const CORBA::Char * cmdline );

        ::SignalObserver::Observer_ptr getObserver(void);

        ::CORBA::Boolean prepare_for_run( const ControlMethod::Method& m );
        ::CORBA::Boolean push_back( SampleBroker::SampleSequence_ptr s );
        ::CORBA::Boolean event_out( CORBA::ULong value );
        ::CORBA::Boolean start_run();
        ::CORBA::Boolean suspend_run();
        ::CORBA::Boolean resume_run();
        ::CORBA::Boolean stop_run();
      
    private:

    };

}
