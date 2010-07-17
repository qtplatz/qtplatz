#include "maincontrollerwindow.h"
#include "ui_maincontrollerwindow.h"

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

MainControllerWindow::MainControllerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainControllerWindow)
{
    ui->setupUi(this);
}

MainControllerWindow::~MainControllerWindow()
{
    delete ui;
}

void
MainControllerWindow::register_device( const ACE_INET_Addr& addr )
{
    std::string addr_str = acewrapper::string( addr );
    std::ostringstream o;
    o << "register_device(" << addr_str << ")" ;
    ui->plainTextEdit->appendPlainText( QString(o.str().c_str() ) );
}

void MainControllerWindow::on_connectButton_clicked()
{

}

void MainControllerWindow::on_MainControllerWindow_destroyed()
{

}

///
void
MainControllerWindow::on_notify_mcast( ACE_Message_Block * mb )
{
   ACE_Message_Block * pfrom = mb->cont();
   ACE_INET_Addr& from_addr( *reinterpret_cast<ACE_INET_Addr *>( pfrom->rd_ptr() ) );
   std::ostringstream o;
   if ( pfrom ) {
      std::string str = acewrapper::string( from_addr );
      o << "[" << str << "]";
   }

   using namespace adportable::protocol;
   using namespace acewrapper;
   LifeCycleData data;
   LifeCycleFrame frame;
   
   lifecycle_frame_serializer::unpack( mb, frame, data );

   try {
       LifeCycle_Hello& hello = boost::get< LifeCycle_Hello& >(data);
       ACE_INET_Addr addr;
       addr.string_to_addr( hello.ipaddr_.c_str() );
       if ( addr.get_ip_address() == 0 ) {
           addr = from_addr;
           addr.set_port_number( hello.portnumber_ );
       }
       register_device( addr );
   } catch ( std::bad_cast& ) {
   }

   o << LifeCycleHelper::to_string( data );
   ui->plainTextEdit->appendPlainText( o.str().c_str() );

   ACE_Message_Block::release( mb );
}

void
MainControllerWindow::on_notify_dgram( ACE_Message_Block * mb )
{
	ACE_Message_Block::release( mb );
}

void
MainControllerWindow::on_notify_timeout( unsigned long sec, long usec )
{
    ACE_UNUSED_ARG(sec);
    ACE_UNUSED_ARG(usec);

/*
	using namespace adportable::protocol;
	if ( mcastHandler_  
		&& ( lifeCycle_.current_state() == LCS_CLOSED || 
		     lifeCycle_.current_state() == LCS_LISTEN ) ) {
	   LifeCycleState state;
	   lifeCycle_.apply_command( adportable::protocol::HELO, state );
	   mcastHandler_->send( ident_.c_str(), ident_.size() + 1 );
	}
*/
}


///////////////////////
void
MainControllerWindow::mcast_init()
{
    acewrapper::instance_manager::initialize();
    
    ACE_Reactor * reactor = acewrapper::TheReactorThread::instance()->get_reactor();
    
    dgramHandler_.reset( new acewrapper::EventHandler< acewrapper::DgramReceiver<QEventReceiver> >() );
    if ( dgramHandler_ ) {
       int port = 6000;
       while ( ! dgramHandler_->open(port++) && port < 6999 )
	  ;
       assert( port < 6999 );
       if ( port < 6999 )
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
