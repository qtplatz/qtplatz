//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "servantplugin.h"
#include "mainwindow.h"

#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <QtCore/qplugin.h>

#include "servantmode.h"
#include <adportable/configuration.h>
#include <adplugin/adplugin.h>
#include <QtCore>
#include <acewrapper/acewrapper.h>
#include <acewrapper/orbservant.h>
//#include <acewrapper/orbmanager.h>
#include <adcontroller/adcontroller.h>
#include <adbroker/adbroker.h>
#include <ace/Thread_Manager.h>

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

	QDir dir = QCoreApplication::instance()->applicationDirPath();
    dir.cdUp();
	dir.cd( "lib/qtPlatz/plugins/ScienceLiaison" );
	QString configFile = dir.path() + "/servant.config.xml";

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
		} 
		if ( name == L"adcontroller" ) {
			adController::initialize( acewrapper::singleton::orbServantManager::instance()->orb() );
			adController::activate();
			adController::run();
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
		if ( it->name() == L"adbroker" )
			mainWindow_->init_debug_adbroker();
		if ( it->name() == L"adcontroller" )
			mainWindow_->init_debug_adcontroller();
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
		if ( name == L"adbroker" )
            adBroker::deactivate();
		if ( name == L"adcontroller" )
			adController::deactivate();
	}
	acewrapper::singleton::orbServantManager::instance()->orb()->shutdown();
	acewrapper::singleton::orbServantManager::instance()->fini();
	ACE_Thread_Manager::instance()->wait();
}

Q_EXPORT_PLUGIN( ServantPlugin )
