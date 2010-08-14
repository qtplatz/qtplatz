//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "servantplugin.h"
#include "mainwindow.h"
#include "servantmode.h"

#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
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

#include <acewrapper/acewrapper.h>
#include <acewrapper/orbservant.h>
#include <acewrapper/constants.h>
#include <qtwrapper/qstring.h>
#include <adportable/string.h>

using namespace servant;
using namespace servant::internal;

ServantPlugin::~ServantPlugin()
{
	delete mainWindow_;
	delete pConfig_;
}

ServantPlugin::ServantPlugin() : mainWindow_(0)
                               , pConfig_(0)
{
}

bool
ServantPlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(arguments);
    Q_UNUSED(error_message);

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
			adBroker::activate();
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
    
    ServantMode * mode = new ServantMode(this);
    if ( mode )
        mode->setContext( context );
    else
        return false;

	mainWindow_ = new MainWindow();

	Core::MiniSplitter * splitter = new Core::MiniSplitter;
	if ( splitter ) {
		splitter->addWidget( mainWindow_ );
		splitter->addWidget( new Core::OutputPanePlaceHolder( mode ) );
        splitter->setStretchFactor( 0, 10 );
        splitter->setStretchFactor( 1, 0 );
		splitter->setOrientation( Qt::Vertical );
	}

	Core::MiniSplitter * splitter2 = new Core::MiniSplitter;
	if ( splitter2 ) {
		splitter2->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) );
        splitter2->addWidget( splitter );
        splitter2->setStretchFactor( 0, 0 );
        splitter2->setStretchFactor( 1, 1 );
	}

    mode->setWidget( splitter2 );

    addAutoReleasedObject(mode);

    mainWindow_->initial_update();
	for ( adportable::Configuration::vector_type::iterator it = config.begin(); it != config.end(); ++it ) {
		if ( it->name() == L"adbroker" ) {
			mainWindow_->init_debug_adbroker();
		} else if ( it->name() == L"adcontroller" ) {
			mainWindow_->init_debug_adcontroller();

			CORBA::Object_var obj;
            //obj = acewrapper::singleton::orbManager::instance()->getObject( acewrapper::constants::adcontroller::manager::name() );
            obj = adplugin::ORBManager::instance()->getObject( acewrapper::constants::adcontroller::manager::name() );
			ControlServer::Manager_var manager = ControlServer::Manager::_narrow( obj );
			if ( ! CORBA::is_nil( manager ) ) {
				ControlServer::Session_var session = manager->getSession( L"debug" );
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
				}
			}
		}
	}
    return true;
}

void
ServantPlugin::extensionsInitialized()
{
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
