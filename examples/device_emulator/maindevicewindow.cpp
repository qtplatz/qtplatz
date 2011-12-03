// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

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
#include <acewrapper/inputcdr.h>
#include <iostream>
#include <QThread>
#include <sstream>

# pragma warning (disable : 4996 )
//# include <ace/OS.h>
# include <ace/Reactor.h>
# include <ace/Message_Block.h>
# include <ace/Task.h>
# include <ace/Reactor_Notification_Strategy.h>
# pragma warning (default : 4996 )

#include "eventreceiver.h"
#include <boost/lexical_cast.hpp>
#include <acewrapper/lifecycle_frame_serializer.h>
#include <sstream>
#include <acewrapper/messageblock.h>
#include "devicefacade.h"

#include "device_averager.h"
#include "device_hvcontroller.h"
#include "./reactor_thread.h"


///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

using namespace device_emulator;

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
MainDeviceWindow::initialize_device_facade()
{
    acewrapper::instance_manager::initialize();
    
    ACE_Reactor * reactor = acewrapper::singleton::ReactorThread::instance()->get_reactor();

    device_emulator::DeviceFacade * facade = device_emulator::singleton::device_facade::instance();
    facade->activate();

    if ( facade->initialize_dgram() )
        reactor->register_handler( facade->get_dgram_handle(), facade, ACE_Event_Handler::READ_MASK );

    if ( facade->initialize_mcast() )
        reactor->register_handler( facade->get_mcast_handle(), facade, ACE_Event_Handler::READ_MASK );

    timerId_ = reactor->schedule_timer( facade, 0, ACE_Time_Value(3), ACE_Time_Value(3) );

    acewrapper::ReactorThread::spawn( acewrapper::singleton::ReactorThread::instance() );
}

void
MainDeviceWindow::closeEvent(QCloseEvent *)
{
    ACE_Reactor * reactor = acewrapper::singleton::ReactorThread::instance()->get_reactor();
    if ( reactor ) {

        DeviceFacade * facade = singleton::device_facade::instance();

        reactor->cancel_timer( facade );
        facade->cancel_timer();
        facade->close_mcast();
        facade->close_dgram();
        facade->close();

        reactor->end_reactor_event_loop();
        reactor->close();
    }
    ACE_Thread_Manager::instance()->wait(); // barrier wait until all threads have shut down.
}

void MainDeviceWindow::on_pushHello_clicked()
{
}

void MainDeviceWindow::on_pushInit_clicked()
{
   ui->pushInit->setEnabled( false );
   ui->dismisButton->setEnabled( true );

   initialize_device_facade();

   DeviceFacade * facade = singleton::device_facade::instance();
   connect( facade, SIGNAL( signal_device_attached( std::string ) ), this, SLOT( handle_device_attached( std::string ) ) );
   connect( facade, SIGNAL( signal_device_detached( std::string ) ), this, SLOT( handle_device_detached( std::string ) ) );
   connect( facade, SIGNAL( signal_debug( QString ) ), this, SLOT( handle_debug( QString ) ) );

   ui->checkBoxAnalyzer->setChecked( true );
   ui->checkBoxAverager->setChecked( true );
}


void MainDeviceWindow::on_dismisButton_clicked()
{
   ui->checkBoxAnalyzer->setChecked( false );
   ui->checkBoxAverager->setChecked( false );

    ACE_Reactor * reactor = acewrapper::singleton::ReactorThread::instance()->get_reactor();
    reactor->cancel_timer( timerId_ );
    reactor->end_reactor_event_loop();
    reactor->close();
}

void
MainDeviceWindow::initial_update()
{
    ui->lineEditStatus->setText( "STATE:Closed" );
    ui->dismisButton->setEnabled( false );
}

void MainDeviceWindow::on_checkBoxAverager_stateChanged(int state)
{
    typedef device_averager TImpl;
    if ( state )
        singleton::device_facade::instance()->attach_device( device_facade_ptr( new device_facade_type(TImpl()) ) );
    else 
        singleton::device_facade::instance()->detach_device( device_facade_type(TImpl()) );
}

void MainDeviceWindow::on_checkBoxIonSource_stateChanged(int state)
{
    ACE_UNUSED_ARG( state );
}

void MainDeviceWindow::on_checkBoxAnalyzer_stateChanged(int state)
{
    typedef device_hvcontroller TImpl;
    if ( state )
        singleton::device_facade::instance()->attach_device( device_facade_ptr( new device_facade_type(TImpl()) ) );
    else 
        singleton::device_facade::instance()->detach_device( device_facade_type(TImpl()) );
}

void
MainDeviceWindow::handle_device_attached( std::string device )
{
    std::ostringstream o;
    o << "device " << device << " has atttached.";
    ui->plainTextEdit->appendPlainText( o.str().c_str() ); 
}

void
MainDeviceWindow::handle_device_detached( std::string device )
{
    std::ostringstream o;
    o << "device " << device << " has detached.";
    ui->plainTextEdit->appendPlainText( o.str().c_str() ); 
}

void
MainDeviceWindow::handle_debug( QString msg )
{
    ui->plainTextEdit->appendPlainText( msg );
}

void MainDeviceWindow::on_pushDisconnect_clicked()
{
    singleton::device_facade::instance()->lifeCycle().force_close();
}

