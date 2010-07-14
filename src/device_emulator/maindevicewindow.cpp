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
#include <acewrapper/inputcdr.h>
#include <ace/Reactor.h>
#include <ace/Message_Block.h>
#include <ace/Task.h>
#include <ace/Reactor_Notification_Strategy.h>
#include <iostream>
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

	ACE_Reactor * reactor = acewrapper::TheReactorThread::instance()->get_reactor();

	dgramHandler_.reset( new acewrapper::EventHandler< acewrapper::DgramReceiver<QEventReceiver> >() );
    if ( dgramHandler_ ) {
		if ( dgramHandler_->open() )
			reactor->register_handler( dgramHandler_.get(), ACE_Event_Handler::READ_MASK );
	}

    mcastHandler_.reset( new acewrapper::EventHandler< acewrapper::McastReceiver<QEventReceiver> >() );
	if ( mcastHandler_ ) {
		if ( mcastHandler_->open() ) 
			reactor->register_handler( mcastHandler_.get(), ACE_Event_Handler::READ_MASK );
	}

	timerHandler_.reset( new acewrapper::EventHandler< acewrapper::TimerReceiver<QEventReceiver> >() );
	if ( timerHandler_ ) {
		timerId_ = reactor->schedule_timer( timerHandler_.get(), 0, ACE_Time_Value(3), ACE_Time_Value(3) );
	}

    acewrapper::ReactorThread::spawn( acewrapper::TheReactorThread::instance() );
}

void
MainDeviceWindow::on_notify_mcast( ACE_Message_Block * mb )
{
	acewrapper::InputCDR cdr( mb );

    unsigned short endian_mark;
    unsigned short protocol_version;
	std::string from;
    size_t size;
	std::vector<char> data;
        
	cdr >> endian_mark;
	cdr >> protocol_version;
	cdr >> from;
	cdr >> size;
    data.resize( size );
	cdr.read( &data[0], size );

	std::ostringstream o;
	o << "mcast endian:" << endian_mark << " octet:" << size << " from:" << from;
	o << " data:" << reinterpret_cast<char *>(&data[0]) << std::endl;

    ui->plainTextEdit->appendPlainText( o.str().c_str() );

	ACE_Message_Block::release( mb );
}

void
MainDeviceWindow::on_notify_dgram( ACE_Message_Block * mb )
{
	acewrapper::InputCDR cdr( mb );

    unsigned short endian_mark;
    unsigned short protocol_version;
	std::string from;
    size_t size;
	std::vector<char> data;
        
	cdr >> endian_mark;
	cdr >> protocol_version;
	cdr >> from;
	cdr >> size;
    data.resize( size );
	cdr.read( &data[0], size );

	std::ostringstream o;
	o << "dgram endian:" << endian_mark << " octet:" << size << " from:" << from;
	o << " data:" << reinterpret_cast<char *>(&data[0]) << std::endl;

	ui->plainTextEdit->appendPlainText( o.str().c_str() );

	ACE_Message_Block::release( mb );
}

void
MainDeviceWindow::on_notify_timeout( unsigned long sec, long usec )
{
    ACE_UNUSED_ARG(sec);
    ACE_UNUSED_ARG(usec);

	using namespace adportable::protocol;
	if ( mcastHandler_  
		&& ( lifeCycle_.current_state() == LCS_CLOSED || 
		     lifeCycle_.current_state() == LCS_LISTEN ) ) {
	   LifeCycleState state;
	   lifeCycle_.apply_command( adportable::protocol::HELO, state );
	   mcastHandler_->send( ident_.c_str(), ident_.size() + 1 );
	}
}

void MainDeviceWindow::on_pushHello_clicked()
{
	if ( mcastHandler_ ) {
		adportable::protocol::LifeCycleState state;
		lifeCycle_.apply_command( adportable::protocol::HELO, state );
		mcastHandler_->send( ident_.c_str(), ident_.size() + 1 );
	}
}

void MainDeviceWindow::on_pushInit_clicked()
{
   mcast_init();

   this->connect( mcastHandler_.get()
	            , SIGNAL( signal_mcast_input( ACE_Message_Block * ) )
                , this, SLOT( on_notify_mcast( ACE_Message_Block* ) ) );

   this->connect( dgramHandler_.get()
                , SIGNAL( signal_dgram_input( ACE_Message_Block * ) )
                , this, SLOT( on_notify_dgram( ACE_Message_Block* ) ) );

   this->connect( timerHandler_.get()
	            , SIGNAL( signal_timeout( unsigned long, long ) )
				, this, SLOT( on_notify_timeout( unsigned long, long ) ) );
}


void MainDeviceWindow::on_dismisButton_clicked()
{
    ACE_Reactor * reactor = acewrapper::TheReactorThread::instance()->get_reactor();
    reactor->cancel_timer( timerId_ );
    reactor->end_reactor_event_loop();
    reactor->close();
}

void MainDeviceWindow::on_MainDeviceWindow_destroyed()
{
    on_dismisButton_clicked();
    // barrier pattern wait should be added here...
}
