// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#include <cstdint>
#include <string>
#include <adinterface/receiverC.h>

namespace adportable    { class Configuration; }
namespace ControlMethod { struct Method; }
namespace SampleBroker  { struct SampleSequenceLine; }
namespace EventLog      { struct LogMessage; }
namespace Instrument    { class Session; }

namespace adcontroller {

    class iTask;
    class oProxy;

    class iProxy {
        iProxy( const iProxy& ) = delete;
        void operator = ( const iProxy& ) = delete;
    public:
        ~iProxy();
        iProxy( iTask& );

        bool initialConfiguration( const adportable::Configuration& );

        // iProxy
        void reset_clock();
        bool connect( const std::string& token );
        bool disconnect();
        bool initialize();
        bool request_shutdown();
        bool eventOut( unsigned long event );

        bool prepare_for_run( const ControlMethod::Method& );
        bool startRun();   // method start
        bool suspendRun(); // method suspend, will hold before next sample load
        bool resumeRun();  // method restart
        bool stopRun();    // method(sequence) stop
       
        uint32_t getStatus();
        Instrument::Session * getSession();

        void objId( unsigned long );
        uint32_t objId() const;

        operator bool() const;

    private:
        class impl;
        impl * impl_;
    };
    
}


