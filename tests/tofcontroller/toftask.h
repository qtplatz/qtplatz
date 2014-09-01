// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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

#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>
#include <vector>
#include <map>
#include <acewrapper/mcasthandler.h>

#pragma warning(disable:4996)
# include <ace/Recursive_Thread_Mutex.h>
# include <ace/Task.h>
# include <ace/Barrier.h>
# include <ace/Message_Queue.h>
# include <adinterface/controlserverC.h>
# include <adinterface/signalobserverC.h>
# include <adinterface/brokerC.h>
#pragma warning(default:4996)

class ACE_Recursive_Thread_Mutex;
class ACE_Notification_Strategy;
class ACE_Reactor;

namespace acewrapper {
    class ReactorThread;
    class McastHandler;
    class TimerHandler;
	template<class T> class ORBServant;
}

namespace TOFInstrument {
    struct ADConfiguration;
    class ADConfigurations;
    struct AnalyzerDeviceData;
}

namespace tofcontroller {

    class DeviceProxy;
    class tofObserver_i;
    class traceObserver_i;

	namespace internal {
		struct observer_events_data;
		struct receiver_data;
	}

	///////////////////////////

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
		void device_update_data( /*argumet to be added */ );
        void controller_update_notification( unsigned long clsid );

        // void session_update_device( boost::any& );
		SignalObserver::Observer_ptr getObserver();
		bool connect ( SignalObserver::ObserverEvents_ptr
			         , SignalObserver::eUpdateFrequency, const std::wstring& );
		bool disconnect( SignalObserver::ObserverEvents_ptr );
        void push_profile_data( ACE_Message_Block * mb );
        void observer_fire_on_update_data( unsigned long objId,  long pos );
        void observer_fire_on_method_changed( unsigned long objId, long pos );
        void observer_fire_on_event( unsigned long objId, unsigned long event, long pos );

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

		typedef std::vector< internal::observer_events_data > observer_events_vector_type;

		typedef std::vector< internal::receiver_data > receiver_vector_type;
		inline receiver_vector_type::iterator ibegin() { return receiver_set_.begin(); }
        inline receiver_vector_type::iterator iend() { return receiver_set_.end(); }

        inline observer_events_vector_type::iterator obegin() { return observer_events_set_.begin(); }
		inline observer_events_vector_type::iterator oend() { return observer_events_set_.end(); }

		// receiver
		receiver_vector_type receiver_set_;

        // observer_events
		observer_events_vector_type observer_events_set_;

		// logger
		Broker::Logger_var logger_;
		std::wstring configXML_;
		boost::scoped_ptr< acewrapper::ReactorThread > reactor_thread_;
		boost::scoped_ptr< acewrapper::McastHandler > mcast_handler_;
		map_type device_proxies_;
		boost::scoped_ptr< TOFInstrument::AnalyzerDeviceData > pAnalyzerDeviceData_;
		boost::scoped_ptr< TOFInstrument::ADConfigurations > pADConfigurations_;
		boost::scoped_ptr< tofObserver_i > pObserver_;
		std::vector< boost::shared_ptr< traceObserver_i > > pTraceObserverVec_;
      };
  
}
