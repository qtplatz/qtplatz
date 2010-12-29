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
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#pragma once

#pragma warning(disable:4996)
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Task.h>
#include <ace/Barrier.h>
#include <ace/Message_Queue.h>
#pragma warning(default:4996)

#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>
#include <adinterface/controlserverC.h>
#include <vector>

class ACE_Recursive_Thread_Mutex;
class ACE_Notification_Strategy;
class ACE_Reactor;

namespace adbroker {

    class BrokerManager;

    class Task : public ACE_Task<ACE_MT_SYNCH>, boost::noncopyable {

        ~Task();
        Task( size_t n_threads = 1 );

    public:  
        inline ACE_Recursive_Thread_Mutex& mutex() { return mutex_; }

        bool open();
        void close();
        bool connect( ControlServer::Session_ptr, Receiver_ptr );
        bool disconnect( ControlServer::Session_ptr, Receiver_ptr );

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

        int handle_timer_timeout( const ACE_Time_Value& tv, const void * arg );
    private:
        friend class BrokerManager;

        ACE_Recursive_Thread_Mutex mutex_;
        ACE_Barrier barrier_;
        size_t n_threads_;
    
        bool internal_disconnect( ControlServer::Session_ptr );
        std::vector<session_data> session_set_;
        std::vector<session_data> session_failed_;
    };


}
