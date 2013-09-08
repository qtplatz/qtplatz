// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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
#include <tofinterface/method.hpp>
//#include <tofinterface/signalC.h>
#include <adinterface/receiverC.h>
#include <adinterface/controlserverC.h>
#include <adinterface/instrumentC.h>
#include <adinterface/signalobserverC.h>
#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <mutex>

namespace Broker { class Logger; }
namespace tofinterface { class tofDATA; class tofProcessedData; struct TraceMetadata; }
namespace EventLog { struct LogMessage; }

namespace tofservant {

    struct observer_events_data;
    struct receiver_data;
    class DeviceFacade;
    class profileObserver_i;
    class traceObserver_i;

    class toftask : boost::noncopyable {
        toftask();
        ~toftask();
        static toftask * instance_;
        static std::mutex mutex_;
    public:
        static toftask * instance();
        bool setConfiguration( const char * );
        bool task_open();
        void task_close();
        bool connect( Receiver_ptr, const std::string& );
        bool disconnect( Receiver_ptr );

        bool initialize();
        unsigned long get_status();
        bool handle_prepare_for_run( ControlMethod::Method );
        bool handle_event_out( unsigned long );
        bool handle_start_run();
        bool handle_suspend_run();
        bool handle_resume_run();
        bool handle_stop_run();

        // internal handler
        void handle_eventlog( EventLog::LogMessage );
        void handle_profile_data( std::shared_ptr< tofinterface::tofDATA > );

        bool setControlMethod( const tof::ControlMethod&, const char * hint );
        
        Instrument::eInstStatus status() const;
        void status( Instrument::eInstStatus );

        void device_update_notification( unsigned long clsid );
        void device_update_data( /*argumet to be added */ );
        void controller_update_notification( unsigned long clsid );

        void deviceSetptChanged( const tof::ControlMethod&, const std::string& );
        bool getControlMethod( tof::ControlMethod& );

        // void session_update_device( boost::any& );
        SignalObserver::Observer_ptr getObserver();
        bool connect ( SignalObserver::ObserverEvents_ptr
                       , SignalObserver::eUpdateFrequency, const std::string& );
        bool disconnect( SignalObserver::ObserverEvents_ptr );

        void push_trace_data( int ch, long pos
                              , const tofinterface::tofProcessedData& data
                              , const tofinterface::TraceMetadata& );

        void observer_fire_on_update_data( unsigned long objId, long pos );
        void observer_fire_on_method_changed( unsigned long objId, long pos );
        void observer_fire_on_event( unsigned long objId, unsigned long event, long pos );

        void session_fire_message( Receiver::eINSTEVENT msg, unsigned long value );
        void session_fire_log( long pri, const std::wstring& format, const std::vector< std::wstring >& args
                               , const std::wstring& msgId = L"");

        void handle_post();
        boost::asio::io_service& io_service() { return io_service_; }

	private:
        void initiate_timer();
        void handle_timeout( const boost::system::error_code& );

        typedef std::vector< observer_events_data > observer_events_vector_type;
        typedef std::vector< receiver_data > receiver_vector_type;

        static const std::size_t interval_ = 1000; // ms

        // receiver
        receiver_vector_type receiver_set_;
        
        // observer_events
        observer_events_vector_type observer_events_set_;
        
        // logger
        Broker::Logger* logger_;

        std::unique_ptr< DeviceFacade > device_facade_;
        std::unique_ptr< profileObserver_i > pObserver_;
        std::vector< std::shared_ptr< traceObserver_i > > pTraceObserverVec_;
        tof::ControlMethod method_;

        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
        boost::asio::deadline_timer timer_;
        std::vector< std::thread > threads_;
    };
    
}
