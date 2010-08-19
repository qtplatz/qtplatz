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
#include <iostream>
#include <QThread>
#include <sstream>

# pragma warning (disable : 4996 )
# include <ace/OS.h>
# include <ace/Reactor.h>
# include <ace/Message_Block.h>
# include <ace/Task.h>
# include <ace/Reactor_Notification_Strategy.h>
# include "../controller/controllerC.h"
# pragma warning (default : 4996 )

#include "eventreceiver.h"
#include <boost/lexical_cast.hpp>
#include <acewrapper/lifecycle_frame_serializer.h>
#include <sstream>
#include <acewrapper/messageblock.h>
#include "devicefacade.h"
// #include <ace/Recursive_Thread_Mutex.h>
// #include <ace/Singleton.h>
#include "roleanalyzer.h"
#include "roleaverager.h"
#include "device_averager.h"
#include "device_hvcontroller.h"

#include "roleesi.h"
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
MainDeviceWindow::mcast_init()
{
    acewrapper::instance_manager::initialize();
    
    ACE_Reactor * reactor = singleton::theReactorThread::instance()->get_reactor();
    
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

			QString qaddr( LifeCycleHelper::to_string( lifeCycleData_hello_ ).c_str() );
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
    
    acewrapper::ReactorThread::spawn( singleton::theReactorThread::instance() );
}

void
MainDeviceWindow::closeEvent(QCloseEvent *)
{
    ACE_Reactor * reactor = singleton::theReactorThread::instance()->get_reactor();
    if ( reactor ) {
        mcastHandler_->close();
        dgramHandler_->close();
        timerHandler_->cancel( reactor, timerHandler_.get() );  // initialize condition
        timerHandler_->wait();
        reactor->end_reactor_event_loop();
        reactor->close();
    }
    ACE_Thread_Manager::instance()->wait(); // barrier wait until all threads have shut down.
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
    acewrapper::scoped_mblock_ptr<> release( mb );

    using namespace adportable::protocol;
    using namespace acewrapper;

    LifeCycleData data;
    LifeCycleFrame frame;
    ACE_InputCDR ace_cdr( mb );
    InputCDR cdr( ace_cdr );
    if ( lifecycle_frame_serializer::unpack( cdr, frame, data ) ) {
        const LifeCycleCommand gotCmd = LifeCycleHelper::command( data );
        if ( gotCmd == CONN_SYN ) {
            ACE_Message_Block * mbfrom = mb->cont();
            if ( mbfrom ) {
                ACE_INET_Addr * remote_addr = reinterpret_cast<ACE_INET_Addr *>( mbfrom->rd_ptr() );
                device_facade::instance()->set_remote_addr( *remote_addr );
            }
        }

        LifeCycleData replyData;
        if ( device_facade::instance()->handle_dgram( frame, data, replyData ) )
            handle_send_dgram( acewrapper::lifecycle_frame_serializer::pack( replyData ) );

        if ( gotCmd == DATA ) {
            dispatch_data( mb );
			/*
            unsigned long classid;
            cdr >> classid;
            std::ostringstream o;
            o << "class id: " << std::hex << classid;

            if ( classid == GlobalConstants::ClassID_IonSourceMethod ) {
                // for firmware debug purpose, here we do not want to use TAO IIOP serializer
                // so just read data from stream.
                unsigned long ionSource;
                cdr >> ionSource;
                o << " type: " << ionSource;
                if ( ionSource == A_Dummy_MSMethod::eIonSource_ESI ) {
                    A_Dummy_MSMethod::ESIMethod m;
                    cdr >> m.needle1_voltage_;
                    cdr >> m.needle2_voltage_;
                    cdr >> m.nebulizing1_flow_;
                    cdr >> m.nebulizing2_flow_;
                    o << " needle " << m.needle1_voltage_ << ", " << m.needle2_voltage_;
                    o << " nebulizer " << m.nebulizing1_flow_ << ", " << m.nebulizing2_flow_;
                } else if ( ionSource == A_Dummy_MSMethod::eIonSource_APCI ) {
                    A_Dummy_MSMethod::APCIMethod m;
                    cdr >> m.needle1_voltage_;
                    cdr >> m.nebulizing1_flow_;
                    o << " nebulizer " << m.nebulizing1_flow_ << " flow " << m.nebulizing1_flow_;
                } else if ( ionSource == A_Dummy_MSMethod::eIonSource_DART ) {
                    A_Dummy_MSMethod::DARTMethod m;
                    cdr >> m.nothing_;
                    o << " dart: nothing " << m.nothing_;
                }
            }
            ui->plainTextEdit->appendPlainText( o.str().c_str() );
			*/
        }
    }
}


void
MainDeviceWindow::dispatch_data( ACE_Message_Block * mb )
{
    ACE_InputCDR in( mb );
	acewrapper::InputCDR cdr( in );

    unsigned long cmdId, clsId;
	cdr >> cmdId;
	cdr >> clsId;
}

void
MainDeviceWindow::on_notify_timeout( unsigned long sec, long usec )
{
    ACE_UNUSED_ARG(sec);
    ACE_UNUSED_ARG(usec);

	using namespace adportable::protocol;
    const LifeCycle& lifeCycle = device_facade::instance()->lifeCycle();
    if ( mcastHandler_ ) {

        if ( lifeCycle.machine_state() == LCS_CLOSED )
            device_facade::instance()->lifeCycleUpdate( HELO );

        if ( lifeCycle.machine_state() == LCS_LISTEN ) {
            // send 'hello' message
            ACE_Message_Block * mb = acewrapper::lifecycle_frame_serializer::pack( LifeCycleData(lifeCycleData_hello_) );
            mcastHandler_->send( mb->rd_ptr(), mb->length() );
            ACE_Message_Block::release( mb );
        }
      
        if ( lifeCycle.machine_state() == LCS_ESTABLISHED ) {
            static size_t n;
            ACE_Message_Block * mb = device_facade::instance()->eventToController( InstrumentStateMachine::event_HeartBeat, n++ );
            dgramHandler_->send( mb->rd_ptr(), mb->length(), device_facade::instance()->get_remote_addr() );
            ACE_Message_Block::release( mb );
        }

    }
}

void MainDeviceWindow::on_pushHello_clicked()
{
    using namespace adportable::protocol;

	if ( mcastHandler_ ) {
        //adportable::protocol::LifeCycleState state;
        //lifeCycle_.apply_command( adportable::protocol::HELO, state );
        //ACE_Message_Block * mb = acewrapper::lifecycle_frame_serializer::pack( LifeCycleData(lifeCycleData_hello_) );
        //mcastHandler_->send( mb->rd_ptr(), mb->length() );
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

   DeviceFacade * facade = device_facade::instance();
   connect( facade, SIGNAL( signal_device_attached( std::string ) ), this, SLOT( handle_device_attached( std::string ) ) );
   connect( facade, SIGNAL( signal_device_detached( std::string ) ), this, SLOT( handle_device_detached( std::string ) ) );
   connect( facade, SIGNAL( signal_send_dgram( ACE_Message_Block * ) ), this, SLOT( handle_send_dgram( ACE_Message_Block * ) ) );
   connect( facade, SIGNAL( signal_debug( QString ) ), this, SLOT( handle_debug( QString ) ) );

   ui->checkBoxAnalyzer->setChecked( true );
}


void MainDeviceWindow::on_dismisButton_clicked()
{
    ACE_Reactor * reactor = singleton::theReactorThread::instance()->get_reactor();
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
    typedef RoleAverager TImpl;
    if ( state )
        device_facade::instance()->attach_device( device_facade_type(TImpl()) );
    else 
        device_facade::instance()->detach_device( device_facade_type(TImpl()) );
}

void MainDeviceWindow::on_checkBoxIonSource_stateChanged(int state)
{
/*
    typedef RoleESI TImpl;
    if ( state )
        device_facade::instance()->attach_device( device_facade_type(TImpl()) );
    else 
        device_facade::instance()->detach_device( device_facade_type(TImpl()) );
*/
}

void MainDeviceWindow::on_checkBoxAnalyzer_stateChanged(int state)
{
    typedef device_hvcontroller TImpl;
    if ( state )
        device_facade::instance()->attach_device( device_facade_type(TImpl()) );
    else 
        device_facade::instance()->detach_device( device_facade_type(TImpl()) );
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
MainDeviceWindow::handle_send_dgram( ACE_Message_Block * mb )
{
    using namespace adportable::protocol;

    if ( device_facade::instance()->lifeCycle().current_state() != LCS_CLOSED ) {
        const ACE_INET_Addr& remote = device_facade::instance()->get_remote_addr();
        dgramHandler_->send( mb->rd_ptr(), mb->length(), remote );
        std::cout << "send dgram to: " << std::string(acewrapper::string( remote )) << std::endl;
    }

    ACE_Message_Block::release( mb );
}

void
MainDeviceWindow::handle_debug( QString msg )
{
    ui->plainTextEdit->appendPlainText( msg );
}

void MainDeviceWindow::on_pushDisconnect_clicked()
{
    using namespace adportable::protocol;

    LifeCycleState state;
    device_facade::instance()->lifeCycle().force_close();
}
