// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef MAINDEVICEWINDOW_H
#define MAINDEVICEWINDOW_H

#include <QMainWindow>
#include <acewrapper/callback.h>
#include <boost/smart_ptr.hpp>
#include "deviceevent.h"

namespace Ui {
    class MainDeviceWindow;
}

class MainDeviceWindow;
class McastServer;
class DeviceTask;
class ACE_Reactor;

class Callback : public acewrapper::Callback, public DeviceEvent {
 public:
  MainDeviceWindow& w_;
  Callback( MainDeviceWindow& w );
  virtual void operator()(const char * pbuf, ssize_t, const ACE_INET_Addr& );
};

class MainDeviceWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainDeviceWindow(QWidget *parent = 0);
    ~MainDeviceWindow();
    void mcast_init();

private:
    Ui::MainDeviceWindow *ui;
    boost::shared_ptr< Callback > callback_;
    boost::shared_ptr< McastServer > mcast_;
      // boost::shared_ptr< DeviceTask > task_;
      // boost::shared_ptr< ACE_Reactor > reactor_;

private slots:
    void on_dismisButton_clicked();
    void on_pushInit_clicked();
    void on_pushHello_clicked();
    void on_notify_mcast();
public slots:
    void on_notify_multicast(const char * pbuf, ssize_t, const ACE_INET_Addr& );
};

#endif // MAINDEVICEWINDOW_H
