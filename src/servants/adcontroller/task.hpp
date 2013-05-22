// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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
#include <adportable/configuration.hpp>

# include <ace/Recursive_Thread_Mutex.h>
# include <ace/Task.h>
# include <ace/Barrier.h>
# include <adinterface/controlserverC.h>

class ACE_Recursive_Thread_Mutex;
class ACE_Notification_Strategy;
class ACE_Reactor;

namespace EventLog { struct LogMessage; }
// namespace adcontroller { namespace constants { enum msg_type; } }

///////////////////////////

namespace adcontroller {

    class iProxy;
    class oProxy;
    class observer_i;

    namespace internal {  struct receiver_data;  }

    class iTask : public ACE_Task<ACE_MT_SYNCH>, boost::noncopyable {
        
        ~iTask();
        iTask( size_t n_threads = 1 );
        static iTask * instance_;
        friend class iTaskManager;
        
    public:  
        inline ACE_Recursive_Thread_Mutex& mutex() { return mutex_; }
        static iTask * instance();
        bool open();
        void close();

	//  instrument communication methods below
        void reset_clock();
	bool initialize();  // initialize hardware 
        bool connect( ControlServer::Session_ptr, Receiver_ptr, const char * token );
        bool disconnect( ControlServer::Session_ptr, Receiver_ptr );
        bool setConfiguration( const wchar_t * xml );
        bool configComplete();
        bool initialize_configuration();
	
	//
	ControlServer::eStatus getStatusCurrent();
	ControlServer::eStatus getStatusBeging(); 
	bool observer_update_data( unsigned long parentId, unsigned long objid, long pos );
	bool observer_update_method( unsigned long parentId, unsigned long objid, long pos );
	bool observer_update_event( unsigned long parentId, unsigned long objid, long pos, unsigned long ev );
	
	typedef std::vector<internal::receiver_data> receiver_vector_type;
	inline receiver_vector_type::iterator receiver_begin() { return receiver_set_.begin(); };
        inline receiver_vector_type::iterator receiver_end()   { return receiver_set_.end(); };
        
        void register_failed( receiver_vector_type::iterator& );
        void commit_failed();
	
	SignalObserver::Observer_ptr getObserver();
        
    private:
        // ACE_Task
        virtual int handle_input( ACE_HANDLE );
        virtual int svc();
        // 
        void doit( ACE_Message_Block * );
        void dispatch ( ACE_Message_Block * );
        
        // int handle_timer_timeout( const ACE_Time_Value& tv, const void * arg );  <-- will handle in iTaskManager

        void handle_dispatch( const EventLog::LogMessage & );
        void handle_dispatch( const ACE_Time_Value& );
        void handle_dispatch_command( ACE_Message_Block * );
	void handle_dispatch( const std::wstring& name, unsigned long msgid, unsigned long value );
        void handle_observer_update_data( unsigned long parentId, unsigned long objId, long pos );
        void handle_observer_update_method( unsigned long parentId, unsigned long objId, long pos );
        void handle_observer_update_events( unsigned long parentId, unsigned long objId, long pos, unsigned long events );

	// 
    public:
	typedef boost::shared_ptr< iProxy > iproxy_ptr;
	typedef boost::shared_ptr< oProxy > oproxy_ptr;
	
	typedef std::vector< boost::shared_ptr<iProxy> > iproxy_vector_type;
	typedef std::vector< boost::shared_ptr<oProxy> > oproxy_vector_type;
	
    private:

        adportable::Configuration config_;
        
        ACE_Recursive_Thread_Mutex mutex_;
        ACE_Barrier barrier_;
        size_t n_threads_;

        bool internal_disconnect( ControlServer::Session_ptr );
        receiver_vector_type receiver_set_;
        receiver_vector_type receiver_failed_;

	std::vector< boost::shared_ptr< iProxy > > iproxies_;
	std::vector< boost::shared_ptr< oProxy > > oproxies_;
	
	boost::shared_ptr< observer_i > pMasterObserver_;
	
	::ControlServer::eStatus status_current_;
	::ControlServer::eStatus status_being_;
    };

} // namespace adcontroller

