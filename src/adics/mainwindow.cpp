//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <adcontroller/adcontroller.h>
#include <ace/Thread_Manager.h>

#include <adinterface/receiverS.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
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
    ACE_Thread_Manager::instance()->wait();
}

bool
MainWindow::initial_update()
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

    session_->connect( _this(), L"debug" );

    this->connect( this
        , SIGNAL( signal_debug_print( long, long, const char * ) )
        , this, SLOT( handle_debug_print( long, long, const char * ) ) );

    return true;
}

void
MainWindow::message( Receiver::eINSTEVENT msg, CORBA::ULong value )
{
  Q_UNUSED(value);
  Q_UNUSED(msg);
}

void
MainWindow::eventLog( const Receiver::LogMessage& )
{
}

void
MainWindow::shutdown()
{
}

void
MainWindow::debug_print( CORBA::Long pri, CORBA::Long cat, const char * text )
{
    static std::string last = text;
    emit signal_debug_print( pri, cat, last.c_str() );
}

void
MainWindow::handle_debug_print( long pri, long cat, const char * text )
{
    ui->plainTextEdit->appendPlainText( text );
}

