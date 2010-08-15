// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Task.h>
#include <ace/Barrier.h>
// #include <ace/Message_Queue.h>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>
#include <adinterface/controlserverC.h>
#include <vector>
#include <adportable/configuration.h>

class ACE_Recursive_Thread_Mutex;
class ACE_Notification_Strategy;
class ACE_Reactor;

namespace EventLog {
    struct LogMessage;
}

///////////////////////////

namespace adcontroller {

    class iBroker : public ACE_Task<ACE_MT_SYNCH>, boost::noncopyable {
        
        ~iBroker();
        iBroker( size_t n_threads = 1 );
        
    public:  
        inline ACE_Recursive_Thread_Mutex& mutex() { return mutex_; }
        
        void reset_clock();
        bool open();
        void close();
        bool connect( ControlServer::Session_ptr, Receiver_ptr );
        bool disconnect( ControlServer::Session_ptr, Receiver_ptr );
        bool setConfiguration( const wchar_t * xml );
        bool configComplete();
        
        struct session_data {
            bool operator == ( const session_data& ) const;
            bool operator == ( const Receiver_ptr ) const;
            bool operator == ( const ControlServer::Session_ptr ) const;
            ControlServer::Session_var session_;
            Receiver_var receiver_;
            session_data() {};
            session_data( const session_data& t ) : session_(t.session_), receiver_(t.receiver_) {};
        };
        
        typedef std::vector<session_data> vector_type;
        inline vector_type::iterator begin() { return session_set_.begin(); };
        inline vector_type::iterator end()   { return session_set_.end(); };
        
        void register_failed( vector_type::iterator& );
        void commit_failed();
        
    private:
        // ACE_Task
        virtual int handle_input( ACE_HANDLE );
        virtual int svc();
        // 
        void doit( ACE_Message_Block * );
        void dispatch ( ACE_Message_Block *, int disp );
        
        // int handle_timer_timeout( const ACE_Time_Value& tv, const void * arg );  <-- will handle in iBrokerManager

        void handle_dispatch( const EventLog::LogMessage & );
        void handle_dispatch( const ACE_Time_Value& );

    private:
        friend class IBrokerManager;

        adportable::Configuration config_;
        
        ACE_Recursive_Thread_Mutex mutex_;
        ACE_Barrier barrier_;
        size_t n_threads_;
        
        bool internal_disconnect( ControlServer::Session_ptr );
        std::vector<session_data> session_set_;
        std::vector<session_data> session_failed_;
    };

} // namespace adcontroller

