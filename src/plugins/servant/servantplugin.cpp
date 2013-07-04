// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "servantplugin.hpp"
#include "servantmode.hpp"
#include "logger.hpp"
#include "outputwindow.hpp"
#include "servantpluginimpl.hpp"

#include <adorbmgr/orbmgr.hpp>

#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <extensionsystem/pluginmanager.h>

# include <ace/Thread_Manager.h>
# include <ace/OS_NS_unistd.h>

#include <acewrapper/acewrapper.hpp>
#include <acewrapper/constants.hpp>
#include <acewrapper/brokerhelper.hpp>

#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/massspectrometer_factory.hpp>
#include <adorbmgr/orbmgr.hpp>
#include <adinterface/instrumentC.h>
#include <adinterface/brokerclientC.h>

#include <adplugin/loader.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adplugin/orbfactory.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/constants.hpp>

#include <adportable/configuration.hpp>
#include <adportable/string.hpp>
#include <adportable/debug.hpp>
#include <adportable/configloader.hpp>

#include <qtwrapper/qstring.hpp>
#include <qtwrapper/application.hpp>

#include <QMessageBox>
#include <QtCore/qplugin.h>
#include <QtCore>
#include <qdebug.h>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <adportable/barrier.hpp>

using namespace servant;
using namespace servant::internal;

ServantPlugin * ServantPlugin::instance_ = 0;

namespace servant { namespace internal {

        struct orbServantCreator {
            std::ostringstream errmsg;
            std::string ior;
            CORBA::Object_var obj;

            adplugin::orbServant * operator()( adplugin::plugin_ptr plugin, std::vector< adplugin::orbServant * >& vec ) {
                if ( ! plugin )
                    return 0;

                adorbmgr::orbmgr * pMgr = adorbmgr::orbmgr::instance();
                
                adplugin::orbFactory * factory = plugin->query_interface< adplugin::orbFactory >();
                if ( ! factory ) {
                    errmsg << plugin->clsid() << " does not support adplugin::orbFactory.";
                    return 0;
                }
                adplugin::orbServant * orbServant = factory->create_instance();
                if ( ! orbServant ) {
                    errmsg << plugin->clsid() << " does not create servant instance.";
                    return 0;                    
                }
                    
                orbServant->initialize( pMgr->orb(), pMgr->root_poa(), pMgr->poa_manager() );
                ior = orbServant->activate();
                if ( ! ior.empty() )
                    obj = pMgr->orb()->string_to_object( ior.c_str() );

                vec.push_back( orbServant );
                return orbServant;
            }
        };

        template<class T> struct findObject {
            static T* find( Broker::Manager_ptr bmgr, const char * name ) {
                CORBA::Object_var obj = bmgr->find_object( name );
                if ( CORBA::is_nil( obj.in() ) )
                    return 0;
                return T::_narrow( obj );
            }
        };

    }
}

ServantPlugin::~ServantPlugin()
{
    final_close();
    delete pImpl_;
    delete pConfig_;
    ACE::fini();
    instance_ = 0;
}

ServantPlugin::ServantPlugin() : pConfig_( 0 )
                               , pImpl_( 0 )
{
    instance_ = this;
    ACE::init();
}

ServantPlugin *
ServantPlugin::instance()
{
    return instance_;
}

bool
ServantPlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(arguments);
    int nErrors = 0;

    do { adportable::debug(__FILE__, __LINE__) << "<----- ServantPlugin::initialize() ..."; } while(0);

    OutputWindow * outputWindow = new OutputWindow;
    addAutoReleasedObject( outputWindow );
    pImpl_ = new internal::ServantPluginImpl( outputWindow );

    ///////////////////////////////////
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
    
    // ACE initialize
    acewrapper::instance_manager::initialize();
    // <------
    
    std::wstring apppath, configFile;
	boost::filesystem::path plugindir;
    do {
        apppath = qtwrapper::application::path( L".." ); // := "~/qtplatz/bin/.."
		configFile = adplugin::loader::config_fullpath( apppath, L"/MS-Cheminformatics/servant.config.xml" );
		plugindir = boost::filesystem::path( configFile ).branch_path();
    } while(0);

	adplugin::loader::populate( plugindir.generic_wstring().c_str() );

	std::vector< adplugin::plugin_ptr > spectrometers;
	if ( adplugin::loader::select_iids( ".*\\.adplugins\\.massSpectrometer\\..*", spectrometers ) ) {
		std::for_each( spectrometers.begin(), spectrometers.end(), []( const adplugin::plugin_ptr& d ){ 
			adcontrols::massspectrometer_factory * factory = d->query_interface< adcontrols::massspectrometer_factory >();
			if ( factory )
				adcontrols::massSpectrometerBroker::register_factory( factory, factory->name() );
		});
	}
    
    const wchar_t * query = L"/ServantConfiguration/Configuration";
    
    pConfig_ = new adportable::Configuration();
    adportable::Configuration& config = *pConfig_;

    adportable::debug( __FILE__, __LINE__ ) << "loading configuration: \"" << configFile << "\"";

    if ( ! adportable::ConfigLoader::loadConfigFile( config, configFile, query ) ) {
        adportable::debug dbg( __FILE__, __LINE__ );
        dbg << "ServantPlugin::initialize loadConfig '" << configFile << "' load failed";
        QMessageBox::warning( 0, dbg.where().c_str(), dbg.str().c_str() );
    }

    // ------------ Broker::Manager initialize first --------------------
    adportable::barrier barrier( 2 );
    adorbmgr::orbmgr * pMgr = adorbmgr::orbmgr::instance();
	if ( pMgr ) {
		pMgr->init( 0, 0 );
		pMgr->spawn( barrier );
	}
	barrier.wait();
    //--------------------------------------------------------------------
    //--------------------------------------------------------------------
    Broker::Manager_var bmgr;
	adplugin::plugin_ptr adbroker = adplugin::loader::select_iid( ".*\\.orbfactory\\.adbroker" );
	if ( adbroker ) {
		internal::orbServantCreator broker_creator;
		adplugin::orbServant * adBroker = broker_creator( adbroker, orbServants_ );
        if ( adBroker ) {
            bmgr = Broker::Manager::_narrow( broker_creator.obj );
            if ( CORBA::is_nil( bmgr ) ) {
                *error_message = "Broker::Manager cannot be created";
                return false;
            }
        } else {
            *error_message = broker_creator.errmsg.str().empty() ? 
                "adplugin for Broker::Manager did not loaded." : broker_creator.errmsg.str().c_str();
            return false;
        }
		adorbmgr::orbmgr::instance()->setBrokerManager( bmgr );
        bmgr->register_ior( adBroker->object_name(), broker_creator.ior.c_str() );
    }
    pImpl_->init_debug_adbroker( bmgr );

    // ----------------------- initialize corba servants ------------------------------
    std::vector< adplugin::plugin_ptr > factories;
    adplugin::loader::select_iids( ".*\\.adplugins\\.orbfactory\\..*", factories );
    for ( const adplugin::plugin_ptr& ptr: factories ) {

        if ( ptr->iid() == adbroker->iid() )
            continue;

        internal::orbServantCreator creator;
        adplugin::orbServant * servant = creator( ptr, orbServants_ );
        if ( servant ) {
            BrokerClient::Accessor_var accessor = BrokerClient::Accessor::_narrow( creator.obj );
            if ( !CORBA::is_nil( accessor ) ) {
                accessor->setBrokerManager( bmgr.in() );
				accessor->adpluginspec( ptr->clsid(), ptr->adpluginspec() );
            }
            try {
                bmgr->register_ior( servant->object_name(), creator.ior.c_str() );
                bmgr->register_object( servant->object_name(), creator.obj );
            } catch ( CORBA::Exception& ex ) {
                *error_message = ex._info().c_str();
                return false;
            }
        }
    }

    using namespace acewrapper::constants;
    ControlServer::Manager_var cmgr ( internal::findObject< ControlServer::Manager >::find( bmgr.in(), adcontroller::manager::_name() ) );
	if ( ! CORBA::is_nil( cmgr ) ) {
        ControlServer::Session_var session = cmgr->getSession( L"debug" );
        if ( ! CORBA::is_nil( session ) )
			pImpl_->init_debug_adcontroller( session );
	}

    // 
    ControlServer::Session_var session;
    std::vector< Instrument::Session_var > i8t_sessions;

    do { adportable::debug(__FILE__, __LINE__) << "<-- ServantPlugin::initialize() ### 3 ##"; } while(0);

    if ( ! CORBA::is_nil( session ) )
        session->configComplete();

    Logger log;
    log( ( nErrors ? L"Servant iitialized with errors" : L"Servernt initialized successfully") );
    
    do { adportable::debug() << "----> ServantPlugin::initialize() completed."; } while(0);
    return true;
}

void
ServantPlugin::extensionsInitialized()
{
    do { adportable::debug(__FILE__, __LINE__) << "ServantPlugin::extensionsInitialized() entered."; } while(0);
    OutputWindow * outputWindow = ExtensionSystem::PluginManager::instance()->getObject< servant::OutputWindow >();
    if ( outputWindow )
        outputWindow->appendLog( L"ServantPlugin::extensionsInitialized()" );
}

ExtensionSystem::IPlugin::ShutdownFlag
ServantPlugin::aboutToShutdown()
{ 
	CORBA::release( pImpl_->manager_ );
	pImpl_->manager_ = 0;
	return SynchronousShutdown;
}

void
ServantPlugin::final_close()
{
	 adportable::debug() << "====== ServantPlugin::final_close servants shutdown... =======";
	// destriction must be reverse order
    for ( orbservant_vector_type::reverse_iterator it = orbServants_.rbegin(); it != orbServants_.rend(); ++it )
        (*it)->deactivate();
	

    adportable::debug() << "====== ServantPlugin::final_close Loggor::shutdown... =======";    
    Logger::shutdown();
    try {
        adportable::debug() << "====== ServantPlugin::final_close orb shutdown... =======";    
		adorbmgr::orbmgr::instance()->shutdown();
        adportable::debug() << "====== ServantPlugin::final_close orb fini... =======";    
        adorbmgr::orbmgr::instance()->fini();
    } catch ( CORBA::Exception& ex ) {
		adportable::debug dbg( __FILE__, __LINE__ );
		dbg << ex._info().c_str();
        QMessageBox::critical( 0, dbg.where().c_str(), dbg.str().c_str() );
	}

    adportable::debug() << "====== ServantPlugin::final_close waiting threads... =======";    
	adorbmgr::orbmgr::instance()->wait();
    adportable::debug() << "====== ServantPlugin::final_close complete =======";    
}

Q_EXPORT_PLUGIN( ServantPlugin )
