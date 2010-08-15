// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adinterface/receiverC.h"
#include "adinterface/controlserverS.h"
#include <acewrapper/orbservant.h>
#include <vector>

namespace adcontroller {

    class session_i : public virtual POA_ControlServer::Session {
    public:
        ~session_i();
        session_i();
      
        ::CORBA::WChar * software_revision();
        ::CORBA::Boolean connect( Receiver_ptr receiver, const CORBA::WChar * token );
        ::CORBA::Boolean disconnect( Receiver_ptr receiver );
		::CORBA::Boolean setConfiguration( const CORBA::WChar * xml );
        ::CORBA::Boolean configComplete();
        ::CORBA::ULong get_status();
        ::CORBA::Boolean initialize();
        ::CORBA::Boolean shutdown();
        ::CORBA::Boolean echo( const CORBA::Char * msg );
        ::CORBA::Boolean shell( const CORBA::Char * cmdline );
        ::SignalObserver::Observer_ptr getObserver();
        ::CORBA::Boolean prepare_for_run( ControlMethod::Method_ptr m );
        ::CORBA::Boolean push_back( SampleBroker::SampleSequence_ptr s );
        ::CORBA::Boolean event_out( CORBA::ULong value );
        ::CORBA::Boolean start_run();
        ::CORBA::Boolean suspend_run();
        ::CORBA::Boolean resume_run();
        ::CORBA::Boolean stop_run();
      
        struct receiver_data {
            bool operator == ( const receiver_data& ) const;
            bool operator == ( const Receiver_ptr ) const;
            bool isTestTool_;
            Receiver_var receiver_;
            receiver_data() : isTestTool_(false) {};
            receiver_data( const receiver_data& t) : isTestTool_(t.isTestTool_), receiver_(t.receiver_) {};
        };
      
        typedef std::vector<receiver_data> vector_type;
      
        inline vector_type::iterator begin() { return receiver_set_.begin(); };
        inline vector_type::iterator end()   { return receiver_set_.end(); };
      
        void register_failed( vector_type::iterator& );
        void commit_failed();
      
    private:
        bool internal_disconnect( Receiver_ptr receiver );
        std::vector<receiver_data> receiver_set_;
        std::vector<receiver_data> receiver_failed_;
    };

}
