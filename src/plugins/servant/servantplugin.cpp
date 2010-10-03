//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "servantplugin.h"
#include "servantmode.h"

#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <extensionsystem/pluginmanager.h>
#include <QtCore/qplugin.h>
#include <QtCore>
#include <ace/Thread_Manager.h>
#include <orbsvcs/CosNamingC.h>

#include <adbroker/adbroker.h>
#include <adcontroller/adcontroller.h>
#include <adinterface/instrumentC.h>

#include <adplugin/adplugin.h>
#include <adplugin/orbLoader.h>
#include <adplugin/orbmanager.h>

#include <adportable/configuration.h>
#include <adportable/string.h>

#include <acewrapper/acewrapper.h>
#include <acewrapper/orbservant.h>
#include <acewrapper/constants.h>
#include <qtwrapper/qstring.h>
#include "outputwindow.h"
#include "servantpluginimpl.h"
#include <QMessageBox>

using namespace servant;
using namespace servant::internal;

ServantPlugin::~ServantPlugin()
{
    delete pImpl_;
	delete pConfig_;
}

ServantPlugin::ServantPlugin() : pConfig_( 0 )
                               , pImpl_( 0 )
{
}

bool
ServantPlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(arguments);
    Q_UNUSED(error_message);

    OutputWindow * outputWindow = new OutputWindow;
    addAutoReleasedObject( outputWindow );
    pImpl_ = new internal::ServantPluginImpl( outputWindow );

    // ACE initialize
	acewrapper::instance_manager::initialize();
	acewrapper::singleton::orbServantManager::instance()->init( 0, 0 );
    // <------

	std::wstring apppath;
	do {
		QDir dir = QCoreApplication::instance()->applicationDirPath();
		dir.cdUp();
		apppath = qtwrapper::wstring::copy( dir.path() );
	} while(0);

	std::wstring configFile = apppath + L"/lib/qtPlatz/plugins/ScienceLiaison/servant.config.xml";

	const wchar_t * query = L"/ServantConfiguration/Configuration";

	pConfig_ = new adportable::Configuration();
	adportable::Configuration& config = *pConfig_;

	adplugin::manager::instance()->loadConfig( config, configFile, query );

	for ( adportable::Configuration::vector_type::iterator it = config.begin(); it != config.end(); ++it ) {
		std::wstring name = it->name();
		std::wstring component = it->component();
		if ( name == L"adbroker" ) {
			adBroker::initialize( acewrapper::singleton::orbServantManager::instance()->orb() );
            if ( ! adBroker::activate() ) {
                QMessageBox mbx;
                mbx.critical( 0, "servantplugin error", "can't activate adBroker servant" );
            }
			adBroker::run();
		} else if ( name == L"adcontroller" ) {
			adController::initialize( acewrapper::singleton::orbServantManager::instance()->orb() );
			adController::activate();
			adController::run();
        } else if ( it->attribute(L"type") == L"orbLoader" ) {
			std::wstring file = apppath + it->module().library_filename();
            it->attribute( L"fullpath", file );
			adplugin::orbLoader& loader = adplugin::manager::instance()->orbLoader( file );
			if ( loader ) {
				loader.initialize( acewrapper::singleton::orbServantManager::instance()->orb() );
				loader.activate();
				loader.run();
			}
        }
	}

    Core::ICore * core = Core::ICore::instance();
    QList<int> context;
    if ( core ) {
        Core::UniqueIDManager * uidm = core->uniqueIDManager();
        if ( uidm ) {
            context.append( uidm->uniqueIdentifier( QLatin1String("Servant.MainView") ) );
            context.append( uidm->uniqueIdentifier( Core::Constants::C_NAVIGATION_PANE ) );
        }
    } else
        return false;

    // 
    ControlServer::Session_var session;
    std::vector< Instrument::Session_var > i8t_sessions;

	for ( adportable::Configuration::vector_type::iterator it = config.begin(); it != config.end(); ++it ) {

		if ( it->name() == L"adbroker" ) {
            pImpl_->init_debug_adbroker( this );
		} else if ( it->name() == L"adcontroller" ) {
            pImpl_->init_debug_adcontroller( this );
			CORBA::Object_var obj;
            obj = adplugin::ORBManager::instance()->getObject( acewrapper::constants::adcontroller::manager::name() );
			ControlServer::Manager_var manager = ControlServer::Manager::_narrow( obj );
			if ( ! CORBA::is_nil( manager ) ) {
                session = manager->getSession( L"debug" );
				session->setConfiguration( it->xml().c_str() );
			}

		} else if ( it->attribute( L"type" ) == L"orbLoader" ) {
			std::string ns_name = adportable::string::convert( it->attribute( L"ns_name" ) );
			if ( ! ns_name.empty() ) {
				CosNaming::Name name;
				name.length(1);
				name[0].id = CORBA::string_dup( ns_name.c_str() );
				CORBA::Object_var obj;
                //obj = acewrapper::singleton::orbManager::instance()->getObject( name );
                obj = adplugin::ORBManager::instance()->getObject( name );
				Instrument::Session_var isession = Instrument::Session::_narrow( obj );
				if ( ! CORBA::is_nil( isession ) ) {
					isession->setConfiguration( it->xml().c_str() );
                    i8t_sessions.push_back( isession );
				}
			}
		}
	}

    if ( ! CORBA::is_nil( session ) ) {
        for ( std::vector< Instrument::Session_var >::iterator it = i8t_sessions.begin(); it != i8t_sessions.end(); ++it )
            (*it)->configComplete();
        session->configComplete();
    }

    return true;
}

void
ServantPlugin::extensionsInitialized()
{
    OutputWindow * outputWindow = ExtensionSystem::PluginManager::instance()->getObject< servant::OutputWindow >();
    outputWindow->appendLog( L"extensionsInitialized()" );
}

void
ServantPlugin::shutdown()
{
	adportable::Configuration& config = *pConfig_;

	for ( adportable::Configuration::vector_type::iterator it = config.begin(); it != config.end(); ++it ) {
		std::wstring name = it->name();
		if ( name == L"adbroker" ) {
            adBroker::deactivate();
		} else if ( name == L"adcontroller" ) {
			adController::deactivate();
        } else if ( it->attribute(L"type") == L"orbLoader" ) {
			std::wstring file = it->attribute( L"fullpath" );
			adplugin::orbLoader& loader = adplugin::manager::instance()->orbLoader( file );
			if ( loader )
				loader.deactivate();
		}
	}
	acewrapper::singleton::orbServantManager::instance()->orb()->shutdown();
	acewrapper::singleton::orbServantManager::instance()->fini();
	ACE_Thread_Manager::instance()->wait();
}




Q_EXPORT_PLUGIN( ServantPlugin )
