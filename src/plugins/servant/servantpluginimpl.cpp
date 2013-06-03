// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include <acewrapper/acewrapper.hpp>
#include <acewrapper/orbservant.hpp>
#include <acewrapper/constants.hpp>
#include <acewrapper/timeval.hpp>
#include <acewrapper/brokerhelper.hpp>

#include <adplugin/manager.hpp>
#include <adplugin/orbmanager.hpp>
#include <adinterface/eventlog_helper.hpp>
#include <qtwrapper/qstring.hpp>

#include "servantpluginimpl.hpp"
#include "servantplugin.hpp"
#include "outputwindow.hpp"

using namespace servant;
using namespace servant::internal;

/////////////////////////////////

ServantPluginImpl::ServantPluginImpl( OutputWindow * p ) : receiver_(*this)
                                                         , logHandler_(*this)
							 , outputWindow_(p)
{
	//adplugin::ORBManager::instance()->init(0, 0);
}

Broker::Manager_ptr
getBrokerManager()
{
	std::string ior = adplugin::manager::instance()->ior( acewrapper::constants::adbroker::manager::_name() );
	CORBA::ORB_var orb = adplugin::ORBManager::instance()->orb();
	return acewrapper::brokerhelper::getManager( orb, ior );
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

		session_->connect( receiver_._this(), "debug" );
	}
}

void
ServantPluginImpl::init_debug_adbroker( ServantPlugin * )
{
    manager_ = getBrokerManager();

	if ( ! CORBA::is_nil( manager_.in() ) ) {

        do {
            Broker::Session_var broker = manager_->getSession( L"debug" );
            broker->connect( "user", "pass", "debug", 0 );
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
        std::wstring wtext = static_cast<wchar_t *>(text);
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

