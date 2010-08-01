// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adinterface/brokerS.h"
#include <deque>
#include <vector>
#include <ace/Recursive_Thread_Mutex.h>

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


