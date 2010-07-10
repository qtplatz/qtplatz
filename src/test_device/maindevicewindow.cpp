//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "maindevicewindow.h"
#include "ui_maindevicewindow.h"
#include <acewrapper/mcastserver.h>
#include <acewrapper/ace_string.h>
#include <acewrapper/acewrapper.h>
#include <ace/Reactor.h>
#include <ace/Message_Block.h>
#include <ace/Task.h>
#include <ace/Reactor_Notification_Strategy.h>
#include <iostream>
#include "deviceevent.h"
#include <QThread>

namespace singleton {
    class TheDevice {
    public:
        static void * thread_entry( void * me ) {
            TheDevice * pThis = reinterpret_cast<TheDevice *>(me);
            if ( pThis ) {
                pThis->reactor_.owner( ACE_OS::thr_self() );
                pThis->run_event_loop();
                pThis->thr_running_ = false;
            }
            return 0;
        }
        inline ACE_Reactor * get_reactor() { return &reactor_; }
    private:
        void run_event_loop() {
            while ( reactor_.reactor_event_loop_done() == 0 )
                reactor_.run_reactor_event_loop();
        }

        TheDevice() : thr_running_(false) {
        }
        ACE_Reactor reactor_;
        bool thr_running_;
        friend class ACE_Singleton<singleton::TheDevice, ACE_Recursive_Thread_Mutex>;
    };
}

typedef ACE_Singleton<singleton::TheDevice, ACE_Recursive_Thread_Mutex> theDevice;

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

   callback_.reset( new Callback(*this) );
   mcast_.reset( new McastServer( *callback_, theDevice::instance()->get_reactor() ) );

   ACE_Thread_Manager::instance()->spawn( ACE_THR_FUNC( singleton::TheDevice::thread_entry )
					  , reinterpret_cast<void *>( theDevice::instance() ) );
}

Callback::Callback( MainDeviceWindow& w ) : w_(w)
{
}

void
Callback::operator ()(const char * pbuf, ssize_t octets, const ACE_INET_Addr& addr )
{
    w_.on_notify_multicast( pbuf, octets, addr );
    emit signal_notify();
}


void MainDeviceWindow::on_notify_multicast( const char * pbuf
					    , ssize_t octets
					    , const ACE_INET_Addr& addr )
{
   Q_UNUSED( octets );
   std::wstring from = acewrapper::string( addr );
   std::wcout << from << L" " << octets << " octets " << pbuf << std::endl;

   
   // QThread::postEvent( *this, new DeviceEvent() );
   // ui->plainTextEdit->appendPlainText( "new mcast message received" );
}

void
MainDeviceWindow::on_notify_mcast()
{
    ui->plainTextEdit->appendPlainText( "new mcast message received" );  
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
   this->connect( callback_.get(), SIGNAL(signal_notify()), this, SLOT(on_notify_mcast()) );
}


void MainDeviceWindow::on_dismisButton_clicked()
{

}
