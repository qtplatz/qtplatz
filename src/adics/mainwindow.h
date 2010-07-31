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

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow, public POA_Receiver {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool initial_update();

protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow *ui;
    ControlServer::Session_var session_;

// POA_Receiver
    void message( Receiver::eINSTEVENT msg, CORBA::ULong value );
    void eventLog( const Receiver::LogMessage& );
    void shutdown();
    void debug_print( CORBA::Long pri, CORBA::Long cat, const char * text );

    private slots:
        void handle_debug_print( long pri, long cat, const char * text );
signals:
        void signal_debug_print( long pri, long cat, const char * text );
};

#endif // MAINWINDOW_H
