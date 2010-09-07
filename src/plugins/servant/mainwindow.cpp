//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <acewrapper/constants.h>
#include <adplugin/orbmanager.h>

#pragma warning(disable:4996)
# include <orbsvcs/CosNamingC.h>
#pragma warning(default:4996)

using namespace servant;
using namespace servant::internal;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , receiver_(*this)
    , logHandler_(*this)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void
MainWindow::initial_update()
{
    adplugin::ORBManager::instance()->init( 0, 0 );
}

void
MainWindow::init_debug_adcontroller()
{
	CORBA::Object_var obj;
    obj = adplugin::ORBManager::instance()->getObject( acewrapper::constants::adcontroller::manager::name() );
	ControlServer::Manager_var manager = ControlServer::Manager::_narrow( obj );
	if ( ! CORBA::is_nil( manager ) ) {
		session_ = manager->getSession( L"debug" );

		connect( this, SIGNAL( signal_debug_print( long, long, QString ) )
			, this, SLOT( handle_debug_print( long, long, QString ) ) );

		session_->connect( receiver_._this(), L"debug" );
	}
}

void
MainWindow::init_debug_adbroker()
{
	CORBA::Object_var obj;
	obj = adplugin::ORBManager::instance()->getObject( acewrapper::constants::adbroker::manager::name() );
    manager_ = Broker::Manager::_narrow( obj.in() );

	if ( ! CORBA::is_nil( manager_.in() ) ) {

        do {
            Broker::Session_var broker = manager_->getSession( L"debug" );
            broker->connect( "user", "pass", "debug" );
        } while(0);

		connect( this, SIGNAL( signal_notify_update( unsigned long ) )
			, this, SLOT( handle_notify_update( unsigned long ) ) );

		Broker::Logger_var logger = manager_->getLogger();

		if ( ! CORBA::is_nil( logger.in() ) ) {
			logger->register_handler( logHandler_._this() );

			Broker::LogMessage msg;
			msg.text = L"init_abroker initialized";
			logger->log( msg );
		}
	}
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

//////////////////////////////////////////////////////////////////
void
Receiver_i::message( Receiver::eINSTEVENT msg, CORBA::ULong value )
{
  Q_UNUSED(value);
  Q_UNUSED(msg);
}

void
Receiver_i::log( const EventLog::LogMessage& )
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
