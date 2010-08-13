// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Task.h>
#include <ace/Barrier.h>
#include <ace/Message_Queue.h>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>
#include <adinterface/controlserverC.h>
#include <adinterface/brokerC.h>
#include <vector>

class ACE_Recursive_Thread_Mutex;
class ACE_Notification_Strategy;
class ACE_Reactor;

namespace tofcontroller {
	
    class i8tTask : public ACE_Task<ACE_MT_SYNCH>, boost::noncopyable {
    public:
        i8tTask( size_t n_threads = 1 );
        ~i8tTask(void);

        inline ACE_Recursive_Thread_Mutex& mutex() { return mutex_; }

        bool open();
        void close();
        bool connect( Receiver_ptr );
        bool disconnect( Receiver_ptr );
	private:
        // ACE_Task
		virtual int handle_input( ACE_HANDLE );
		virtual int svc();
		// <--
        void doit( ACE_Message_Block * );
        int handle_timer_timeout( const ACE_Time_Value& tv, const void * arg );
        void initialize();

    private:
        ACE_Recursive_Thread_Mutex mutex_;
        ACE_Barrier barrier_;
        size_t n_threads_;

        struct receiver_data {
            bool operator == ( const receiver_data& ) const;
            bool operator == ( const Receiver_ptr ) const;
            receiver_data() {};
            receiver_data( const receiver_data& t ) : receiver_(t.receiver_) {}
            Receiver_var receiver_;
        };
        typedef std::vector< receiver_data > vector_type;
        inline vector_type::iterator begin() { return receiver_set_.begin(); }
        inline vector_type::iterator end() { return receiver_set_.end(); }

        vector_type receiver_set_;
		Broker::Logger_var logger_;
    };
  
}
