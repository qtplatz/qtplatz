// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include <QObject>

# include <adinterface/receiverS.h>
# include <adinterface/loghandlerS.h>
# include <adinterface/controlserverC.h>
# include <adinterface/brokerC.h>

/////////////////

namespace adplugin {
    class QBrokerSessionEvent;
}

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

        ///////////////////////////////////////////////////////////
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
            friend class Receiver_i;
            friend class LogHandler_i;
            void signal_debug_print( long pri, long cat, QString );
            void signal_notify_update( unsigned long logId );

        };

    }
}
