/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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

#include "adicontroller_global.hpp"
#include "eventlog.hpp"
#include <memory>

namespace adicontroller {

    class Receiver;

#if defined _MSC_VER
    ADICONTROLLERSHARED_TEMPLATE_EXPORT template class ADICONTROLLERSHARED_EXPORT std::weak_ptr < Receiver > ;
#endif

    class ADICONTROLLERSHARED_EXPORT Receiver : public std::enable_shared_from_this< Receiver > {
    public:

        virtual ~Receiver();
        Receiver();
        
        enum eMSGPRIORITY {
            pri_DEBUG
            , pri_INFO     // informational
            , pri_ERROR    // user correctable error
            , pri_EXCEPT   // exception, can be recoverd. Mostly a bug
            , pri_PANIC // non-recoverable exception
        };
        
        enum eINSTEVENT {
            NOTHING
            , HEARTBEAT // formerly, it was timer signal
            , CONFIG_CHANGED
            , STATE_CHANGED
            , METHOD_RECEIVED
            , PREPARE_FOR_RUN
            , START_IN           // a.k.a. METHOD_RUN triger in
            , START_OUT
            , INJECT_IN
            , INJECT_OUT
            , EVENT_IN
            , EVENT_OUT
            , SETPTS_UPDATED
            , ACTUAL_UPDATED
            , PROTOCOL_OVERRIDE_ENABLED // it is in tuning mode
            , CLIENT_ATTACHED
        };

        virtual void message( eINSTEVENT msg, uint32_t value ) = 0; // send message to client
        virtual void log( const EventLog::LogMessage& log );
        virtual void shutdown();
        virtual void debug_print( uint32_t priority, uint32_t category, const std::string& text );
    };

}
