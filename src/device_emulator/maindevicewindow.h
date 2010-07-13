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

private:
    Ui::MainDeviceWindow *ui;
    // boost::shared_ptr< acewrapper::EventHandler<DeviceEvent> > devEvent_;
    // boost::shared_ptr< McastServer > mcast_;

    boost::shared_ptr< acewrapper::EventHandler< acewrapper::DgramReceiver<QEventReceiver> > > dgramHandler_;
    boost::shared_ptr< acewrapper::EventHandler< acewrapper::McastReceiver<QEventReceiver> > > mcastHandler_;
    boost::shared_ptr< acewrapper::EventHandler< acewrapper::TimerReceiver<QEventReceiver> > > timerHandler_;

    std::string ident_;
      unsigned long timerId_;

private slots:
    void on_MainDeviceWindow_destroyed();
    void on_dismisButton_clicked();
    void on_pushInit_clicked();
    void on_pushHello_clicked();
    void on_notify_mcast( ACE_Message_Block * mb );
    void on_notify_timeout( const ACE_Time_Value * );
};

#endif // MAINDEVICEWINDOW_H
