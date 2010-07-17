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
#include <boost/lexical_cast.hpp>
#include <acewrapper/lifecycle_frame_serializer.h>
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
}

void
MainDeviceWindow::mcast_init()
{
    acewrapper::instance_manager::initialize();
    
    ACE_Reactor * reactor = acewrapper::TheReactorThread::instance()->get_reactor();
    
    dgramHandler_.reset( new acewrapper::EventHandler< acewrapper::DgramReceiver<QEventReceiver> >() );
    if ( dgramHandler_ ) {

        int port = 7000;
        while ( ! dgramHandler_->open(port++) && port < 7999 )
            ;
        assert( port < 7999 );
        if ( port < 7999 ) {
            reactor->register_handler( dgramHandler_.get(), ACE_Event_Handler::READ_MASK );

            lifeCycleData_hello_.portnumber_ = static_cast<const ACE_INET_Addr>(*dgramHandler_).get_port_number();
            lifeCycleData_hello_.ipaddr_ = acewrapper::string( static_cast<const ACE_INET_Addr>(*dgramHandler_) );
            lifeCycleData_hello_.device_name_ = "emulator";
            lifeCycleData_hello_.manufacturer_ = "TH";
            lifeCycleData_hello_.proto_ = "udp";
            lifeCycleData_hello_.revision_ = "1.0";
            lifeCycleData_hello_.serial_number_ = boost::lexical_cast< std::string >(ACE_OS::getpid());

            using namespace adportable::protocol;

            QString qaddr( LifeCycle_Hello::to_string( lifeCycleData_hello_ ).c_str() );
            ui->lineEditStatus->setText( qaddr );
        }
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
    ACE_Message_Block * pfrom = mb->cont();
    std::ostringstream o;
    if ( pfrom )
        o << "[" << std::string(acewrapper::string( *reinterpret_cast<ACE_INET_Addr *>(pfrom->rd_ptr()) )).c_str() << "]";

    using namespace adportable::protocol;
    using namespace acewrapper;
    LifeCycleData data;
    LifeCycleFrame frame;
    
    lifecycle_frame_serializer::unpack( mb, frame, data );

    o << LifeCycleHelper::to_string( data );

    ui->plainTextEdit->appendPlainText( o.str().c_str() );

	ACE_Message_Block::release( mb );
}

void
MainDeviceWindow::on_notify_dgram( ACE_Message_Block * mb )
{
    ACE_Message_Block * pfrom = mb->cont();
    std::string fromaddr;
    if ( pfrom )
        fromaddr = acewrapper::string( *reinterpret_cast<ACE_INET_Addr *>(pfrom->rd_ptr()) );

    // ui->plainTextEdit->appendPlainText( o.str().c_str() );

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

       ACE_Message_Block * mb = acewrapper::lifecycle_frame_serializer::pack( LifeCycleData(lifeCycleData_hello_) );
       mcastHandler_->send( mb->rd_ptr(), mb->length() );
	}
}

void MainDeviceWindow::on_pushHello_clicked()
{
    using namespace adportable::protocol;

	if ( mcastHandler_ ) {
		adportable::protocol::LifeCycleState state;
		lifeCycle_.apply_command( adportable::protocol::HELO, state );
        ACE_Message_Block * mb = acewrapper::lifecycle_frame_serializer::pack( LifeCycleData(lifeCycleData_hello_) );
        mcastHandler_->send( mb->rd_ptr(), mb->length() );
	}
}

void MainDeviceWindow::on_pushInit_clicked()
{
   ui->pushInit->setEnabled( false );
   ui->dismisButton->setEnabled( true );

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

void
MainDeviceWindow::initial_update()
{
    ui->lineEditStatus->setText( "STATE:Closed" );
    ui->dismisButton->setEnabled( false );
}

void MainDeviceWindow::on_checkBoxAverager_stateChanged(int )
{

}

void MainDeviceWindow::on_checkBoxIonSource_stateChanged(int )
{

}

void MainDeviceWindow::on_checkBoxAnalyzer_stateChanged(int )
{

}
