// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef MAINDEVICEWINDOW_H
#define MAINDEVICEWINDOW_H

#include <QMainWindow>
// #include <acewrapper/callback.h>
#include <boost/smart_ptr.hpp>
#include <adportable/protocollifecycle.h>

namespace Ui {
    class MainDeviceWindow;
}

namespace acewrapper {
    template<class T> class EventHandler;
    class DgramHandler;
    class McastHandler;
    class TimerHandler;
    template<class T> class DgramReceiver;
    template<class T> class McastReceiver;
    template<class T> class TimerReceiver;
}

// class DeviceEvent;
// class McastServer;

class MainDeviceWindow;
class ACE_Reactor;
class ACE_Time_Value;
class ACE_INET_Addr;
class ACE_Message_Block;

class QEventReceiver;

class MainDeviceWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainDeviceWindow(QWidget *parent = 0);
    ~MainDeviceWindow();
    void mcast_init();
    void initial_update();

protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::MainDeviceWindow *ui;
    // adportable::protocol::LifeCycle lifeCycle_;

    // device will handle one unicast data gram
    boost::shared_ptr< acewrapper::EventHandler< acewrapper::DgramReceiver<QEventReceiver> > > dgramHandler_;

    // mcast dgram, which is a counterpart to controller
    boost::shared_ptr< acewrapper::EventHandler< acewrapper::McastReceiver<QEventReceiver> > > mcastHandler_;
    boost::shared_ptr< acewrapper::EventHandler< acewrapper::TimerReceiver<QEventReceiver> > > timerHandler_;

	unsigned long timerId_;
    adportable::protocol::LifeCycleFrame lifeCycleFrame_hello_;
    adportable::protocol::LifeCycle_Hello lifeCycleData_hello_;

private slots:
    void on_pushDisconnect_clicked();
    void on_checkBoxAnalyzer_stateChanged(int );
    void on_checkBoxIonSource_stateChanged(int );
    void on_checkBoxAverager_stateChanged(int );
    void on_dismisButton_clicked();
    void on_pushInit_clicked();
    void on_pushHello_clicked();
    void on_notify_mcast( ACE_Message_Block * mb );
    void on_notify_dgram( ACE_Message_Block * mb );
    void on_notify_timeout( unsigned long, long );

    // device_facade notifications
    void handle_device_attached( std::string device );
    void handle_device_detached( std::string device );
    void handle_send_dgram( ACE_Message_Block * );
    void handle_debug( QString );
};

#endif // MAINDEVICEWINDOW_H
