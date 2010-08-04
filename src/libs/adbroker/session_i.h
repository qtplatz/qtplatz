// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#pragma warning (disable: 4996)
# include "adinterface/brokerS.h"
#pragma warning (default: 4996)

namespace broker {

    class session_i : public POA_Broker::Session {
        PortableServer::ObjectId oid_;
    public:
        void oid( const PortableServer::ObjectId& oid ) { oid_ = oid; }
        const PortableServer::ObjectId& oid() { return oid_; }

        session_i(void);
        ~session_i(void);
        bool connect( const char * user, const char * pass, const char * token );
    };

}