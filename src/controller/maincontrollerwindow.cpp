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
#include "treemodel.h"
#include "deviceproxy.h"
#include "devicetext.h"

using namespace adportable::protocol;
using namespace acewrapper;

MainControllerWindow::MainControllerWindow(QWidget *parent) :  QMainWindow(parent)
                                                            , ui(new Ui::MainControllerWindow)
{
    ui->setupUi(this);
}

MainControllerWindow::~MainControllerWindow()
{
    delete ui;
}

void
MainControllerWindow::closeEvent(QCloseEvent * ev)
{
    ACE_Reactor * reactor = acewrapper::TheReactorThread::instance()->get_reactor();
    if ( reactor ) {
        if ( mcastHandler_ )
            mcastHandler_->close();
        if ( timerHandler_ ) {
            timerHandler_->cancel( reactor, timerHandler_.get() );  // initialize condition
            timerHandler_->wait();
        }
        reactor->end_reactor_event_loop();
    }
    ACE_Thread_Manager::instance()->wait(); // barrier wait until all threads have shut down.
}

void
MainControllerWindow::on_initial_update()
{
    treeModel_.reset( new TreeModel( this ) );
    ui->treeView->setModel( treeModel_.get() );

    for (int column = 0; column < treeModel_->columnCount(); ++column)
        ui->treeView->resizeColumnToContents(column);
    // ui->tabWidget->clear();
}

void
MainControllerWindow::multicast_update_device( const ACE_INET_Addr& addr
                                              , const LifeCycleFrame& frame
                                              , const LifeCycleData& data )
{
    std::string addr_str = acewrapper::string( addr );

    map_type::iterator it = devices_.find( addr_str );
    if ( it == devices_.end() ) { // || boost::get<const LifeCycle_Hello *>(&data) ) {
        devices_[ addr_str ] = boost::shared_ptr< DeviceProxy >( new DeviceProxy( addr ) );

        connect( devices_[ addr_str ].get()
            , SIGNAL( signal_dgram_to_device(std::string, QString, QString) )
            , this, SLOT( handle_dgram_to_device(std::string, QString, QString) ) ); 

        DeviceText * pDevText = new DeviceText(this);
        ui->tabWidget->addTab( pDevText, addr_str.c_str() );

        connect( devices_[ addr_str ].get()
            , SIGNAL( signal_debug(std::string, QString) )
            , pDevText, SLOT( handle_debug(std::string, QString) ) ); 

        TreeModel * model = treeModel_.get();
        int row = model->rowCount();
        model->insertRow( row );
        QModelIndex parentIndex = model->index( row, 0 );
        model->setData( parentIndex, addr_str.c_str() );
        model->setData( model->index( row, 2 ), LifeCycleHelper::to_string( data ).c_str() );
    }
    devices_[ addr_str ]->mcast_update_device( frame, data );

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
       multicast_update_device( addr, frame, data );
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

    ACE_Time_Value tv( sec, usec );

	// lock
	std::map< std::string, boost::shared_ptr< DeviceProxy > >::const_iterator it;
	for ( it = devices_.begin(); it != devices_.end(); ++it )
		it->second->notify_timeout( tv );
    // unlock

}

void
MainControllerWindow::handle_dgram_to_device( std::string remote_addr, QString local_address, QString description )
{
    TreeModel& model = *treeModel_;
    int row = model.findParent( remote_addr.c_str() );
    if ( row >= 0 ) {
        QModelIndex index = model.index( row, 0 );
        int childRow = model.rowCount( index );
        model.insertRow( childRow, index );
        model.setData( model.index( childRow, 0, index ), local_address );
        model.setData( model.index( childRow, 2, index ), description );
    }
}

void
MainControllerWindow::handle_debug( std::string remote_addr, QString description )
{
}


///////////////////////
void
MainControllerWindow::mcast_init()
{
    acewrapper::instance_manager::initialize();

    ACE_Reactor * reactor = acewrapper::TheReactorThread::instance()->get_reactor();
    mcastHandler_.reset( new acewrapper::EventHandler< acewrapper::McastReceiver<QEventReceiver> >() );
    if ( mcastHandler_ ) {
       if ( mcastHandler_->open() ) 
           reactor->register_handler( mcastHandler_.get(), ACE_Event_Handler::READ_MASK );
    }

    timerHandler_.reset( new acewrapper::EventHandler< acewrapper::TimerReceiver<QEventReceiver> >() );
    if ( timerHandler_ ) {
        timerId_ = reactor->schedule_timer( timerHandler_.get(), 0, ACE_Time_Value(3), ACE_Time_Value(3) );
    }

    connect( mcastHandler_.get()
           , SIGNAL( signal_mcast_input( ACE_Message_Block * ) )
		   , this, SLOT( on_notify_mcast( ACE_Message_Block* ) ) );

    connect( timerHandler_.get()
		   , SIGNAL( signal_timeout( unsigned long, long ) )
		   , this, SLOT( on_notify_timeout( unsigned long, long ) ) );

    acewrapper::ReactorThread * pThread = acewrapper::TheReactorThread::instance();
    acewrapper::ReactorThread::spawn( pThread );
}


void MainControllerWindow::on_actionInitialize_triggered()
{
    mcast_init();
}
