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
#include <map>
#include <acewrapper/mcasthandler.h>

class ACE_Recursive_Thread_Mutex;
class ACE_Notification_Strategy;
class ACE_Reactor;

namespace acewrapper {
    class ReactorThread;
    class McastHandler;
    class TimerHandler;
}

namespace TOFInstrument {
    struct ADConfiguration;
    class ADConfigurations;
    struct AnalyzerDeviceData;
}

namespace tofcontroller {

    class DeviceProxy;
	
    class TOFTask : public ACE_Task<ACE_MT_SYNCH>, boost::noncopyable {
    public:
        TOFTask( size_t n_threads = 4 );
        ~TOFTask(void);

        inline ACE_Recursive_Thread_Mutex& mutex() { return mutex_; }

        bool setConfiguration( const wchar_t * );
        bool open();
        void close();
        bool connect( Receiver_ptr );
        bool disconnect( Receiver_ptr );

		void adConfiguration( const TOFInstrument::ADConfigurations& );
		bool adConfiguration( TOFInstrument::ADConfigurations& config ) const;

		void setAnalyzerDeviceData( const TOFInstrument::AnalyzerDeviceData& );
		bool getAnalyzerDeviceData( TOFInstrument::AnalyzerDeviceData& ) const;
        void device_update_notification( unsigned long clsid );
        void controller_update_notification( unsigned long clsid );

        // void session_update_device( boost::any& );

	private:
        // ACE_Task
		virtual int handle_input( ACE_HANDLE );
		virtual int handle_timeout( const ACE_Time_Value& tv, const void * arg );
		virtual int svc();
		// <--
        void doit( ACE_Message_Block * );
        void internal_initialize();
		bool internal_initialize_reactor();
		bool internal_initialize_timer();
		bool internal_initialize_mcast();

        void dispatch_command( ACE_Message_Block * );
        void dispatch_debug( ACE_Message_Block * );
        void dispatch_mcast( ACE_Message_Block * );
        void dispatch_dgram( ACE_Message_Block * );
        void dispatch_sendto_device( const ACE_Message_Block * );
        void dispatch_query_device( const ACE_Message_Block * );
        void command_initialize();

    public:
        void dispatch_debug( const std::wstring& text, const std::wstring& key );
		typedef std::map< std::wstring, boost::shared_ptr< DeviceProxy > > map_type;

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
		std::wstring configXML_;
		boost::scoped_ptr< acewrapper::ReactorThread > reactor_thread_;
		boost::scoped_ptr< acewrapper::McastHandler > mcast_handler_;
		map_type device_proxies_;
		boost::scoped_ptr< TOFInstrument::AnalyzerDeviceData > pAnalyzerDeviceData_;
		boost::scoped_ptr< TOFInstrument::ADConfigurations > pADConfigurations_;
    };
  
}
