// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <QObject>

#pragma warning(disable:4996)
#pragma warning(disable:4805)
# include <adinterface/receiverS.h>
# include <adinterface/loghandlerS.h>
#pragma warning(default:4805)
# include <adinterface/controlserverC.h>
# include <adinterface/brokerC.h>
#pragma warning(default:4996)

/////////////////
namespace servant {

    class OutputWindow;
    class ServantPlugin;

    namespace internal {
        
        class ServantPluginImpl;

        ///////////////////////////////////////////////////////////
        class LogHandler_i : public POA_LogHandler {
            ServantPluginImpl& impl_;
        public:
            LogHandler_i( ServantPluginImpl& t ) : impl_(t) {}
            void notify_update( CORBA::ULong );
        };
        
        ///////////////////////////////////////////////////////////
        class Receiver_i : public POA_Receiver {
            ServantPluginImpl& impl_;
        public:
            Receiver_i( ServantPluginImpl& t ) : impl_(t) {}
            // POA_Receiver
            void message( Receiver::eINSTEVENT msg, CORBA::ULong value );
            void log( const EventLog::LogMessage& );
            void shutdown();
            void debug_print( CORBA::Long pri, CORBA::Long cat, const char * text );
        };

        ////
        class ServantPluginImpl : public QObject {
            Q_OBJECT

        public:
            ServantPluginImpl( OutputWindow * p );

            void init_debug_adcontroller( ServantPlugin * );
            void init_debug_adbroker( ServantPlugin * );

            ControlServer::Session_var session_;
            Broker::Manager_var manager_;

            internal::Receiver_i receiver_;
            internal::LogHandler_i logHandler_;
            OutputWindow * outputWindow_;

        private slots:
            void handle_debug_print( long pri, long cat, QString );
            void handle_notify_update( unsigned long lofId );

        signals:
            friend Receiver_i;
            friend LogHandler_i;
            void signal_debug_print( long pri, long cat, QString );
            void signal_notify_update( unsigned long logId );

        };

    }
}
