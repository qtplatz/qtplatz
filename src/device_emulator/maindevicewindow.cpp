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
#include <acewrapper/eventhandler.h>
#include <acewrapper/dgramhandler.h>
#include <acewrapper/mcasthandler.h>
#include <acewrapper/timerhandler.h>
#include <ace/Reactor.h>
#include <ace/Message_Block.h>
#include <ace/Task.h>
#include <ace/Reactor_Notification_Strategy.h>
#include <iostream>
#include "deviceevent.h"
#include <QThread>
#include <sstream>
#include <ace/OS.h>
#include "eventreceiver.h"

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
}

void
MainDeviceWindow::mcast_init()
{
    acewrapper::instance_manager::initialize();
    
    std::ostringstream o;
    o << "HELLO:device_emulator:";
    o << "S/N=" << ACE_OS::getpid() << ",";
    ident_ = o.str();

    //devEvent_.reset( new acewrapper::EventHandler<DeviceEvent>() );

    dgramHandler_.reset( new acewrapper::EventHandler< acewrapper::DgramReceiver<QEventReceiver> >() );
    if ( dgramHandler_ )
        dgramHandler_->open();
    mcastHandler_.reset( new acewrapper::EventHandler< acewrapper::McastReceiver<QEventReceiver> >() );
    if ( mcastHandler_ )
        mcastHandler_->open();
    timerHandler_.reset( new acewrapper::EventHandler< acewrapper::TimerReceiver<QEventReceiver> >() );

    ACE_Reactor * reactor = acewrapper::TheReactorThread::instance()->get_reactor();
    reactor->register_handler( dgramHandler_.get(), ACE_Event_Handler::READ_MASK );
    reactor->register_handler( mcastHandler_.get(), ACE_Event_Handler::READ_MASK );

    timerId_ = reactor->schedule_timer( timerHandler_.get(), 0, ACE_Time_Value(3), ACE_Time_Value(3) );

    // mcast_.reset( new McastServer( *devEvent_, reactor ) );
    // devEvent_->timerId_ = reactor->schedule_timer( devEvent_.get(), 0, ACE_Time_Value(3), ACE_Time_Value(3) );

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

void
MainDeviceWindow::on_notify_timeout( const ACE_Time_Value * )
{
    ui->plainTextEdit->appendPlainText( "timeout..." );
    if ( mcastHandler_ )
        mcastHandler_->send( ident_.c_str(), ident_.size() + 1 );
/*
    if ( mcast_ )
        mcast_->send( ident_.c_str(), ident_.size() + 1 );
*/
}

void MainDeviceWindow::on_pushHello_clicked()
{
    if ( mcastHandler_ )
        mcastHandler_->send( ident_.c_str(), ident_.size() + 1 );
/*
   if ( mcast_ )
       mcast_->send( ident_.c_str(), ident_.size() + 1 );
*/
}

void MainDeviceWindow::on_pushInit_clicked()
{
   mcast_init();
/**
   this->connect( devEvent_.get()
                , SIGNAL(signal_mcast(const char *, int, const ACE_INET_Addr*))
                , this
                , SLOT( on_notify_mcast(const char*, int, const ACE_INET_Addr*) ) );

   this->connect( devEvent_.get()
                , SIGNAL(signal_timeout(const ACE_Time_Value *))
                , this
                , SLOT( on_notify_timeout(const ACE_Time_Value *) ) );
**/
}


void MainDeviceWindow::on_dismisButton_clicked()
{
    ACE_Reactor * reactor = acewrapper::TheReactorThread::instance()->get_reactor();
    // reactor->cancel_timer( devEvent_->timerId_ );
    reactor->end_reactor_event_loop();
    reactor->close();
}

void MainDeviceWindow::on_MainDeviceWindow_destroyed()
{

}
