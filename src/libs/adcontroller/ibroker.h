// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
#include <adportable/configuration.h>

#pragma warning(disable:4996)
# include <ace/Recursive_Thread_Mutex.h>
# include <ace/Task.h>
# include <ace/Barrier.h>
# include <adinterface/controlserverC.h>
#pragma warning(default:4996)

class ACE_Recursive_Thread_Mutex;
class ACE_Notification_Strategy;
class ACE_Reactor;

namespace EventLog {
    struct LogMessage;
}

///////////////////////////

namespace adcontroller {

    class iProxy;
    class oProxy;
	class observer_i;

	namespace internal {
        struct session_data;
	}

    class iBroker : public ACE_Task<ACE_MT_SYNCH>, boost::noncopyable {
        
        ~iBroker();
        iBroker( size_t n_threads = 1 );
        
    public:  
        inline ACE_Recursive_Thread_Mutex& mutex() { return mutex_; }
        bool open();
        void close();

		//  instrument communication methods below
        void reset_clock();
		bool initialize();  // initialize hardware 
        bool connect( ControlServer::Session_ptr, Receiver_ptr, const wchar_t * token );
        bool disconnect( ControlServer::Session_ptr, Receiver_ptr );
        bool setConfiguration( const wchar_t * xml );
        bool configComplete();

		//
		ControlServer::eStatus getStatusCurrent();
		ControlServer::eStatus getStatusBeging(); 
		bool observer_update_data( unsigned long parentId, unsigned long objid, long pos );
		bool observer_update_method( unsigned long parentId, unsigned long objid, long pos );
		bool observer_update_event( unsigned long parentId, unsigned long objid, long pos, unsigned long ev );

		typedef std::vector<internal::session_data> session_vector_type;
		inline session_vector_type::iterator session_begin() { return session_set_.begin(); };
        inline session_vector_type::iterator session_end()   { return session_set_.end(); };
        
        void register_failed( session_vector_type::iterator& );
        void commit_failed();

		SignalObserver::Observer_ptr getObserver();
        
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
		void handle_dispatch( const std::wstring& name, unsigned long msgid, unsigned long value );
        void handle_observer_update_data( unsigned long parentId, unsigned long objId, long pos );
        void handle_observer_update_method( unsigned long parentId, unsigned long objId, long pos );
        void handle_observer_update_events( unsigned long parentId, unsigned long objId, long pos, unsigned long events );

    private:
        friend class IBrokerManager;

        adportable::Configuration config_;
        
        ACE_Recursive_Thread_Mutex mutex_;
        ACE_Barrier barrier_;
        size_t n_threads_;

        bool internal_disconnect( ControlServer::Session_ptr );
		session_vector_type session_set_;
		session_vector_type session_failed_;

		// 
		typedef boost::shared_ptr< iProxy > iproxy_ptr;
		typedef boost::shared_ptr< oProxy > oproxy_ptr;

		typedef std::vector< boost::shared_ptr<iProxy> > iproxy_vector_type;
		typedef std::vector< boost::shared_ptr<oProxy> > oproxy_vector_type;

		std::vector< boost::shared_ptr< iProxy > > iproxies_;
		std::vector< boost::shared_ptr< oProxy > > oproxies_;

		boost::shared_ptr< observer_i > pMasterObserver_;

		::ControlServer::eStatus status_current_;
		::ControlServer::eStatus status_being_;
    };

} // namespace adcontroller

