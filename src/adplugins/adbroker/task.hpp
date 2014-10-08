// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include <compiler/diagnostic_push.h>
#include <compiler/disable_deprecated.h>
#include <adinterface/brokerC.h>
#include <adinterface/brokereventC.h>
#include <compiler/diagnostic_pop.h>
#include <workaround/boost/asio.hpp>
#include <adportable/asio/thread.hpp>
#include <thread>
#include <vector>
#include <map>
#include <mutex>

class ACE_Notification_Strategy;
class ACE_Reactor;

namespace adcontrols {
    class MassSpectrum;
}

namespace portfolio {
    class Portfolio;
    class Folium;
}

namespace adbroker {

    class BrokerManager;

    class Task : boost::noncopyable {

        Task();
    public:  
        ~Task();

        inline std::mutex& mutex() { return mutex_; }

        int task_open();
        int task_close();
        bool connect( Broker::Session_ptr, BrokerEventSink_ptr, const char * token );
        bool disconnect( Broker::Session_ptr, BrokerEventSink_ptr );

        struct session_data {
            bool operator == ( const session_data& ) const;
            bool operator == ( const BrokerEventSink_ptr ) const;
            bool operator == ( const Broker::Session_ptr ) const;
            Broker::Session_var session_;
            BrokerEventSink_var receiver_;
            std::string token_;
            session_data() {};
            session_data( const session_data& t ) : session_(t.session_), receiver_(t.receiver_), token_(t.token_) {};
        };

        typedef std::vector<session_data> vector_type;
        inline vector_type::iterator begin() { return session_set_.begin(); };
        inline vector_type::iterator end()   { return session_set_.end(); };
      
        void register_failed( vector_type::iterator& );
        void commit_failed();

    public:
        inline boost::asio::io_service& io_service() { return io_service_; }
        void handleCoaddSpectrum( const std::wstring& token, SignalObserver::Observer_ptr observer, double x1, double x2 );

	private:
		void internal_coaddSpectrum( const std::wstring& token, const adcontrols::MassSpectrum& );
		void appendOnFile( const std::wstring& filename, const adcontrols::MassSpectrum&, const std::wstring& title );

    private:
        friend class BrokerManager;

        bool internal_disconnect( Broker::Session_ptr );

        // 
        void initiate_timer();
        void handle_timeout( const boost::system::error_code& );

        std::mutex mutex_;

        std::vector<session_data> session_set_;
        std::vector<session_data> session_failed_;

        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
        boost::asio::deadline_timer timer_;
        std::vector< adportable::asio::thread > threads_;
        std::size_t interval_;
    };


}
