//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <adcontroller/adcontroller.h>
#include <adbroker/adbroker.h>
#include <ace/Thread_Manager.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    receiver_(*this),
    logHandler_(*this)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void
MainWindow::closeEvent( QCloseEvent * )
{
    adcontroller::abort_server();
    adbroker::abort_server();
    ACE_Thread_Manager::instance()->wait();
}

bool
MainWindow::init_adcontroller()
{
    adcontroller::run(0, 0);

    CORBA::ORB_ptr orb = adcontroller::orb();
    if ( CORBA::is_nil(orb) )
        return false;

    std::string ior = adcontroller::ior();
    CORBA::Object_var obj = orb->string_to_object( ior.c_str() );
    
    if ( CORBA::is_nil( obj.in() ) )
        return false;

    session_ = ControlServer::Session::_narrow( obj.in() );
    if ( CORBA::is_nil( session_.in() ) )
        return false;

    connect( this, SIGNAL( signal_debug_print( long, long, QString ) )
           , this, SLOT( handle_debug_print( long, long, QString ) ) );

    session_->connect( receiver_._this(), L"debug" );
    return true;
}

bool
MainWindow::init_adbroker()
{
    adbroker::run(0, 0);

    CORBA::ORB_ptr orb = adbroker::orb();
    if ( CORBA::is_nil(orb) )
        return false;

    std::string ior = adbroker::ior();
    CORBA::Object_var obj = orb->string_to_object( ior.c_str() );
    
    if ( CORBA::is_nil( obj.in() ) )
        return false;

    manager_ = Broker::Manager::_narrow( obj.in() );
    if ( CORBA::is_nil( manager_.in() ) )
        return false;

    connect( this, SIGNAL( signal_notify_update( unsigned long ) )
           , this, SLOT( handle_notify_update( unsigned long ) ) );

    Broker::Logger_var logger = manager_->getLogger();

    if ( ! CORBA::is_nil( logger.in() ) ) {
        logger->register_handler( logHandler_._this() );

        Broker::LogMessage msg;
        msg.text = L"init_abroker initialized";
        logger->log( msg );
    }
    return true;
}

bool
MainWindow::initial_update()
{
    init_adbroker();
    init_adcontroller();
    return true;
}

void
MainWindow::invoke_debug_print( long pri, long cat, QString text )
{
    emit signal_debug_print( pri, cat, text );
}

void
MainWindow::invoke_notify_update( unsigned long logId )
{
    emit signal_notify_update( logId );
}

void
MainWindow::handle_debug_print( long pri, long cat, QString text )
{
    Q_UNUSED(pri);
    Q_UNUSED(cat);
    ui->plainTextEdit->appendPlainText( text );
}

void
MainWindow::handle_notify_update( unsigned long logid )
{
    Broker::Logger_var logger = manager_->getLogger();
    if ( CORBA::is_nil( logger.in() ) )
        return;
    Broker::LogMessage msg;
    if ( logger->findLog( logid, msg ) ) {
        CORBA::WString_var text = logger->to_string( msg );
        std::wstring wtext = text;
        std::string stext( wtext.length(), ' ' );
        std::copy( wtext.begin(), wtext.end(), stext.begin() );
        ui->plainTextEdit->appendPlainText( stext.c_str() );
    }
}


//////////////////////////////////////////////////////////////////
void
Receiver_i::message( Receiver::eINSTEVENT msg, CORBA::ULong value )
{
  Q_UNUSED(value);
  Q_UNUSED(msg);
}

void
Receiver_i::eventLog( const Receiver::LogMessage& )
{
}

void
Receiver_i::shutdown()
{
}

void
Receiver_i::debug_print( CORBA::Long pri, CORBA::Long cat, const char * text )
{
    mainWindow_.invoke_debug_print( pri, cat, QString( text ) );
}

/////////////////////////////////////////////////////////////////////////////

void
LogHandler_i::notify_update( CORBA::ULong logId )
{
    mainWindow_.invoke_notify_update( logId );
}