// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <utils/fancymainwindow.h>

#pragma warning(disable:4996)
#pragma warning(disable:4805)
# include <adinterface/receiverS.h>
#pragma warning(default:4805)
# include <adinterface/controlserverC.h>
# include <adinterface/brokerC.h>
# include <adinterface/loghandlerS.h>
#pragma warning(default:4996)

namespace Ui {
    class MainWindow;
}

namespace servant {
    namespace internal {
        
        class MainWindow;
        
        ///////////////////////////////////////////////////////////
        class LogHandler_i : public POA_LogHandler {
            MainWindow& mainWindow_;
        public:
            LogHandler_i( MainWindow& t ) : mainWindow_(t) {}
            
            // POA_LogHandler
            void notify_update( CORBA::ULong );
        };
        
        ///////////////////////////////////////////////////////////
        class Receiver_i : public POA_Receiver {
            MainWindow& mainWindow_;
        public:
            Receiver_i( MainWindow& t ) : mainWindow_(t) {}
            
            // POA_Receiver
            void message( Receiver::eINSTEVENT msg, CORBA::ULong value );
            void log( const EventLog::LogMessage& );
            void shutdown();
            void debug_print( CORBA::Long pri, CORBA::Long cat, const char * text );
        };
        
        ///////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////
                           
        class MainWindow : public QMainWindow { // public Utils::FancyMainWindow
            Q_OBJECT

        public:
            explicit MainWindow(QWidget *parent = 0);
            ~MainWindow();
                    
            void initial_update();
            void init_debug_adcontroller();
            void init_debug_adbroker();
            
            void invoke_debug_print( long pri, long cat, QString );
            void invoke_notify_update( unsigned long logId );
            
        private:
            Ui::MainWindow *ui;
            ControlServer::Session_var session_;
            Broker::Manager_var manager_;
            Receiver_i receiver_;
            LogHandler_i logHandler_;
            
        private slots:
            void handle_debug_print( long pri, long cat, QString );
            void handle_notify_update( unsigned long lofId );
        signals:
            void signal_debug_print( long pri, long cat, QString );
            void signal_notify_update( unsigned long logId );
        };

    }
}

#endif // MAINWINDOW_H
