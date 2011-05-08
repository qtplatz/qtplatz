// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <deque>
#include <vector>
#pragma warning (disable: 4996)
# include "adinterface/brokerS.h"
# include <ace/Recursive_Thread_Mutex.h>
#pragma warning (default: 4996)

namespace broker {

    class logger_i : public POA_Broker::Logger {
        PortableServer::POA_ptr poa_;
        PortableServer::ObjectId oid_;
    public:
        logger_i( PortableServer::POA_ptr poa );
        ~logger_i(void);
        void oid( const PortableServer::ObjectId& oid ) { oid_ = oid; }
        const PortableServer::ObjectId& oid() { return oid_; }

        void log( const Broker::LogMessage& );
        bool findLog( CORBA::ULong logId, Broker::LogMessage& msg );
        bool nextLog( Broker::LogMessage& msg );
        CORBA::WChar * to_string( const Broker::LogMessage& msg );

        bool register_handler( LogHandler_ptr );
        bool unregister_handler( LogHandler_ptr );

        struct handler_data {
            bool operator == ( const handler_data& ) const;
            bool operator == ( const LogHandler_ptr ) const;
            LogHandler_var handler_;
            handler_data() {};
            handler_data( const handler_data& t) : handler_(t.handler_) {};
        };

    private:
        unsigned long logId_;
        std::deque< Broker::LogMessage > log_;
        ACE_Recursive_Thread_Mutex mutex_;

        typedef std::vector<handler_data> vector_type;
        inline vector_type::iterator begin() { return handler_set_.begin(); };
        inline vector_type::iterator end()   { return handler_set_.end(); };

        bool internal_disconnect( LogHandler_ptr );
        std::vector<handler_data> handler_set_;

    };

}


