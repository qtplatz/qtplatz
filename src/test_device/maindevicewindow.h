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

class MainDeviceWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainDeviceWindow(QWidget *parent = 0);
    ~MainDeviceWindow();
    void mcast_init();

private:
    Ui::MainDeviceWindow *ui;
    boost::shared_ptr< DeviceEvent > devEvent_;
    boost::shared_ptr< McastServer > mcast_;

private slots:
    void on_dismisButton_clicked();
    void on_pushInit_clicked();
    void on_pushHello_clicked();
    void on_notify_mcast(const char *, int, const ACE_INET_Addr* );
};

#endif // MAINDEVICEWINDOW_H
