#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <adinterface/instrumentC.h>
#include <adinterface/ReceiverS.h>


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
                 , POA_Receiver {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setSession( Instrument::Session_ptr );

private:
        // POA_Receiver
		void message( ::Receiver::eINSTEVENT msg, CORBA::ULong value );
        void log( const EventLog::LogMessage& );
        void shutdown();
        void debug_print( CORBA::Long pri, CORBA::Long cat, const char * text );
        // <-------------

private:
    Ui::MainWindow *ui;
    Instrument::Session_var isession_;

private slots:
    void on_pushButton_inject_clicked(bool checked);
    void on_pushButton_connect_clicked(bool checked);
    void on_pushButton_start_stop_clicked(bool checked);
};

#endif // MAINWINDOW_H
