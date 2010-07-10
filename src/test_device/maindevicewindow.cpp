//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "maindevicewindow.h"
#include "ui_maindevicewindow.h"
#include <acewrapper/mcastserver.h>
#include <acewrapper/ace_string.h>
#include <acewrapper/acewrapper.h>
#include <acewrapper/reactorthread.h>
#include <ace/Reactor.h>
#include <ace/Message_Block.h>
#include <ace/Task.h>
#include <ace/Reactor_Notification_Strategy.h>
#include <iostream>
#include "deviceevent.h"
#include <QThread>
#include <sstream>

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

MainDeviceWindow::MainDeviceWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainDeviceWindow)
{
    ui->setupUi(this);
}

MainDeviceWindow::~MainDeviceWindow()
{
    delete ui;
    acewrapper::instance_manager::dispose();
}

void
MainDeviceWindow::mcast_init()
{
    acewrapper::instance_manager::initialize();

    devEvent_.reset( new DeviceEvent() );
    mcast_.reset( new McastServer( *devEvent_, acewrapper::TheReactorThread::instance()->get_reactor() ) );
    acewrapper::ReactorThread::spawn( acewrapper::TheReactorThread::instance() );
}

void
MainDeviceWindow::on_notify_mcast( const char * pbuf, int octets, const ACE_INET_Addr* addr )
{
    std::string from = acewrapper::string( ACE_INET_Addr(*addr) );
    std::ostringstream o;
    o << "1:" << pbuf << " " << octets << " octets received " << std::string(from);
    ui->plainTextEdit->appendPlainText( o.str().c_str() );
}

void MainDeviceWindow::on_pushHello_clicked()
{
   if ( mcast_ ) {
      mcast_->send( "hello", sizeof("hello") );
   }
}

void MainDeviceWindow::on_pushInit_clicked()
{
   mcast_init();

   this->connect( devEvent_.get()
                , SIGNAL(signal_mcast(const char *, int, const ACE_INET_Addr*))
                , this
                , SLOT( on_notify_mcast(const char*, int, const ACE_INET_Addr*) ) );

}


void MainDeviceWindow::on_dismisButton_clicked()
{

}
