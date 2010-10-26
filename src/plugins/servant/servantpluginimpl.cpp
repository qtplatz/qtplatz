//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include <acewrapper/acewrapper.h>
#include <acewrapper/orbservant.h>
#include <acewrapper/constants.h>
#include <acewrapper/timeval.h>
#include <acewrapper/brokerhelper.h>

#include <adplugin/adplugin.h>
#include <adplugin/orbLoader.h>
#include <adplugin/orbmanager.h>
#include <adinterface/eventlog_helper.h>
#include <qtwrapper/qstring.h>

#include "servantpluginimpl.h"
#include "servantplugin.h"
#include "outputwindow.h"

using namespace servant;
using namespace servant::internal;

/////////////////////////////////

ServantPluginImpl::ServantPluginImpl( OutputWindow * p ) : outputWindow_(p)
                                                         , receiver_(*this)
                                                         , logHandler_(*this)
{
    adplugin::ORBManager::instance()->init(0, 0);
}

Broker::Manager_ptr
getBrokerManager()
{
	std::string ior = adplugin::manager::instance()->ior( acewrapper::constants::adbroker::manager::_name() );
    return acewrapper::brokerhelper::getManager( adplugin::ORBManager::instance()->orb(), ior );
}

void
ServantPluginImpl::init_debug_adcontroller( ServantPlugin * )
{
	Broker::Manager_var mgr = getBrokerManager();
	CORBA::Object_var obj;
	if ( ! CORBA::is_nil( mgr ) ) {
		std::string ior = mgr->ior( acewrapper::constants::adcontroller::manager::_name() );
        if ( ! ior.empty() )
			obj = adplugin::ORBManager::instance()->string_to_object( ior );
	}

	ControlServer::Manager_var manager = ControlServer::Manager::_narrow( obj );
	if ( ! CORBA::is_nil( manager ) ) {
		session_ = manager->getSession( L"debug" );

		connect( this, SIGNAL( signal_debug_print( long, long, QString ) )
			, this, SLOT( handle_debug_print( long, long, QString ) ) );

		session_->connect( receiver_._this(), L"debug" );
	}
}

void
ServantPluginImpl::init_debug_adbroker( ServantPlugin * )
{
    manager_ = getBrokerManager();

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
            msg.tv_sec = msg.tv_usec = 0;
			msg.text = L"init_abroker initialized";
			logger->log( msg );
		}
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void
ServantPluginImpl::handle_debug_print( long pri, long cat, QString text )
{
    Q_UNUSED(pri);
    Q_UNUSED(cat);
    outputWindow_->appendLog( text );
}

void
ServantPluginImpl::handle_notify_update( unsigned long logid )
{
    Broker::Logger_var logger = manager_->getLogger();
    if ( CORBA::is_nil( logger.in() ) )
        return;
    Broker::LogMessage msg;
    if ( logger->findLog( logid, msg ) ) {
        CORBA::WString_var text = logger->to_string( msg );
        std::wstring wtext = text;
        outputWindow_->appendLog( wtext );
    }
}


/////////////////////////////////////////////////////////////////////////////

void
LogHandler_i::notify_update( CORBA::ULong logId )
{
    emit impl_.signal_notify_update( logId );
}

void
Receiver_i::message( Receiver::eINSTEVENT msg, CORBA::ULong value )
{
    Q_UNUSED(value);
    Q_UNUSED(msg);
}

void
Receiver_i::log( const EventLog::LogMessage& log )
{
    using namespace adinterface::EventLog;

    std::wstring text = LogMessageHelper::toString( log );
    QString qtext = acewrapper::to_string( log.tv.sec, log.tv.usec ).c_str();
    qtext += "\t: ";
    qtext += qtwrapper::qstring::copy( text );

    emit impl_.signal_debug_print( log.priority, 0, qtext );
}

void
Receiver_i::shutdown()
{
}

void
Receiver_i::debug_print( CORBA::Long pri, CORBA::Long cat, const char * text )
{
    Q_UNUSED( pri );
    Q_UNUSED( cat );
    emit impl_.signal_debug_print( pri, cat, QString( text ) );
}

