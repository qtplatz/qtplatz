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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <adportable/configuration.hpp>
#include <string>
#include <boost/noncopyable.hpp>

#include <adinterface/receiverS.h>
#include <adinterface/instrumentC.h>

namespace ControlMethod {
    struct Methohd;
}

namespace SampleBroker {
	struct SampleSequenceLine;
}

namespace adcontroller {

    class iTask;
    class oProxy;

    class iProxy : public POA_Receiver, boost::noncopyable {
    public:
        iProxy( iTask& );

        bool initialConfiguration( const adportable::Configuration& );

        // POA_Receiver
        void message( ::Receiver::eINSTEVENT msg, CORBA::ULong value );
        void log( const EventLog::LogMessage& );
        void shutdown();
        void debug_print( CORBA::Long pri, CORBA::Long cat, const char * text );

        // iProxy
        void reset_clock();
        bool connect( const std::string& token );
        bool disconnect();
        bool initialize();
        bool request_shutdown();
        bool eventOut( unsigned long event );

        bool prepare_for_run( const SampleBroker::SampleSequenceLine&
                              , const ControlMethod::Method& );
        bool startRun();   // method start
        bool suspendRun(); // method suspend, will hold before next sample load
        bool resumeRun();  // method restart
        bool stopRun();    // method(sequence) stop
       
        unsigned long getStatus();
        Instrument::Session_ptr getSession();
        void objId( unsigned long );
        unsigned long objId() const;
        inline operator bool () const { return objref_; }

    private:
        bool objref_;
        unsigned long objId_;
        Instrument::Session_var impl_;
        iTask& task_;
        adportable::Configuration config_;
        std::wstring name_;
    };
    
}


