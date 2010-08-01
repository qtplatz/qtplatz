// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <adinterface/receiverS.h>
#include <adinterface/controlserverC.h>
#include <adinterface/brokerC.h>
#include <adinterface/loghandlerS.h>

namespace Ui {
    class MainWindow;
}

class MainWindow;

class LogHandler_i : public POA_LogHandler {
    MainWindow& mainWindow_;
public:
    LogHandler_i( MainWindow& t ) : mainWindow_(t) {}

    // POA_LogHandler
    void notify_update( CORBA::ULong );
};

class Receiver_i : public POA_Receiver {
    MainWindow& mainWindow_;
public:
    Receiver_i( MainWindow& t ) : mainWindow_(t) {}

    // POA_Receiver
    void message( Receiver::eINSTEVENT msg, CORBA::ULong value );
    void eventLog( const Receiver::LogMessage& );
    void shutdown();
    void debug_print( CORBA::Long pri, CORBA::Long cat, const char * text );
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool initial_update();

protected:
    void closeEvent(QCloseEvent *);

private:
    bool init_adcontroller();
    bool init_adbroker();

private:
    Ui::MainWindow *ui;
    ControlServer::Session_var session_;
    Broker::Manager_var manager_;
    Receiver_i receiver_;
    LogHandler_i logHandler_;
public:
    void invoke_debug_print( long pri, long cat, QString );
    void invoke_notify_update( unsigned long logId );

private slots:
    void handle_debug_print( long pri, long cat, QString );
    void handle_notify_update( unsigned long logId );

signals:
    void signal_debug_print( long pri, long cat, QString );
    void signal_notify_update( unsigned long logId );
};

#endif // MAINWINDOW_H
