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

#include "servantplugin.hpp"
#include "servantmode.hpp"
#include "logger.hpp"
#include "outputwindow.hpp"
#include "servantpluginimpl.hpp"
#include "orbservantmanager.hpp"

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

#include <adbroker/adbroker.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontroller/adcontroller.hpp>
#include <adinterface/instrumentC.h>
#include <adinterface/brokerclientC.h>

#include <adplugin/loader.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/orbmanager.hpp>
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
#include <boost/foreach.hpp>

using namespace servant;
using namespace servant::internal;

ServantPlugin::~ServantPlugin()
{
    final_close();
    delete pImpl_;
    delete pConfig_;
    ACE::fini();
}

ServantPlugin::ServantPlugin() : pConfig_( 0 )
                               , pImpl_( 0 )
{
    ACE::init();
}

namespace servant {

    namespace internal {

        // struct adbroker_initializer {
        //     servant::ORBServantManager * pMgr_;
        //     std::wstring file_;
        //     std::string ior_;

        //     adbroker_initializer( servant::ORBServantManager * p, const std::wstring& file )
        //         : pMgr_(p), file_(file) {
        //     }

        //     bool operator()( adportable::Configuration::vector_type::iterator it ) {
        //         adplugin::orbLoader& loader = adplugin::manager::instance()->orbLoader( file_ );
        //         if ( loader ) {
        //             loader.initialize( pMgr_->orb(), pMgr_->root_poa(), pMgr_->poa_manager() );
        //             ior_ = loader.activate();
        //             if ( ! ior_.empty() )
        //                 adplugin::manager::instance()->register_ior(
        //                     acewrapper::constants::adbroker::manager::_name(), ior_ );
        //             return true;
        //         } else {
        //             it->attribute( L"loadstatus", L"failed" );
        //             return false;
        //         }
        //     }

        //     Broker::Manager_ptr getObject() {
        //         // ---- private naming service (Broker::Manager) start ----
        //         CORBA::Object_var obj = pMgr_->orb()->string_to_object( ior_.c_str() );
        //         Broker::Manager_var mgr = Broker::Manager::_narrow( obj );
        //         if ( CORBA::is_nil( mgr ) )
        //             return 0;
        //         // store broker's ior to self
        //         for ( size_t nTrial = 0; nTrial < 25 ; ++nTrial ) {
        //             try {
        //                 mgr->register_ior( acewrapper::constants::adbroker::manager::_name(), ior_.c_str() );
        //                 break;
        //             } catch ( CORBA::Exception& ex ) {
        //                 adportable::debug( __FILE__, __LINE__ ) << "CORBA::Exception: " << ex._info().c_str();
        //                 if ( nTrial > 5 )
        //                     QMessageBox::warning(0, "ServantPlugin - Broker::Manager::registor_ior", ex._info().c_str() );
        //                 else
        //                     QMessageBox::critical(0, "ServantPlugin - Broker::Manager::registor_ior", ex._info().c_str() );
        //                 ACE_OS::sleep(0);
        //             }
        //         }
        //         return mgr._retn();
        //     }
        // };


        /* configuration finder */
        struct config_finder {
            adportable::Configuration& config_;
            config_finder( adportable::Configuration& config) : config_( config ) {
            }
            adportable::Configuration::vector_type::iterator operator()( const std::wstring& name ) {
                for ( adportable::Configuration::vector_type::iterator it = config_.begin(); it != config_.end(); ++it ) {
                    if ( it->name() == name )
                        return it;
                }
                return config_.end();
            }
        };
    }
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
    boost::barrier barrier( 2 );
    servant::ORBServantManager * pMgr = servant::ORBServantManager::instance();
	if ( pMgr ) {
		pMgr->init( 0, 0 );
		pMgr->spawn( barrier );
	}
	barrier.wait();
    adplugin::ORBManager::instance()->initialize( pMgr->orb(), pMgr->root_poa() );

    //--------------------------------------------------------------------
    std::string iorBroker;
    adplugin::plugin_ptr adbroker = adplugin::loader::select_iid( ".*\\.orbfactory\\.adbroker" );
    if ( adbroker ) {
        adplugin::orbFactory * factory = adbroker->query_interface< adplugin::orbFactory >();
        if ( ! factory  ) {
            *error_message = "adplugin::adBroker does not provide orbFactory interface";
            return false;
        }
        adplugin::orbServant * adBroker;
        if ( adBroker = factory->create_instance() ) {
            adBroker->initialize( pMgr->orb(), pMgr->root_poa(), pMgr->poa_manager() );
            iorBroker = adBroker->activate();
            if ( iorBroker.empty() ) {
                *error_message = "adBroker object could not be activated";
                return false;
            }

            // for deactivation
            orbServants_.push_back( adBroker );  

            using acewrapper::constants::adbroker::manager;
            adplugin::manager::instance()->register_ior( manager::_name(), iorBroker );
            CORBA::Object_var obj = pMgr->orb()->string_to_object( iorBroker.c_str() );
            broker_manager_ = Broker::Manager::_narrow( obj );
            if ( CORBA::is_nil( broker_manager_ ) ) {
                *error_message = "Broker::Manager cannot be created";
                return false;
            }
            try {
                broker_manager_->register_ior( manager::_name(), iorBroker.c_str() );
            } catch ( CORBA::Exception& ex ) {
                *error_message = ex._info().c_str();
                return false;
            }
        }
    }
    try {
        pImpl_->init_debug_adbroker( this );
    } catch ( std::exception& ex ) {
        adportable::debug(__FILE__, __LINE__) << ex.what();
    }

    // ----------------------- initialize corba servants ------------------------------
    do {
        std::vector< adplugin::plugin_ptr > factories;
        adplugin::loader::select_iids( ".*\\.orbfactory\\..*$", factories );
        BOOST_FOREACH( const adplugin::plugin_ptr& ptr, factories ) {
            if ( ptr->iid() == adbroker->iid() )
                continue;
            adportable::debug(__FILE__, __LINE__ ) << "initializing " << ptr->iid() << ptr->clsid();
            adplugin::orbFactory * factory = ptr->query_interface< adplugin::orbFactory >();
            if ( factory  ) {
                adplugin::orbServant * servant = factory->create_instance();
                if ( servant ) {

                    orbServants_.push_back( servant );

                    servant->initialize( pMgr->orb(), pMgr->root_poa(), pMgr->poa_manager() );
                    std::string ior = servant->activate();
                    CORBA::Object_var obj = pMgr->orb()->string_to_object( ior.c_str() );
                    BrokerClient::Accessor_var accessor = BrokerClient::Accessor::_narrow( obj );
                    if ( !CORBA::is_nil( accessor ) )
                        accessor->setBrokerManager( broker_manager_.in() );
                }
            } else {
                *error_message += QString( ptr->iid() ) + " does not provide orbFactory interface";
            }
        }
    } while( 0 );

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

    // 
    ControlServer::Session_var session;
    std::vector< Instrument::Session_var > i8t_sessions;

#if 0
    for ( adportable::Configuration::vector_type::iterator it = config.begin(); it != config.end(); ++it ) {
        if ( it->name() == L"adbroker" ) {
            try {
                pImpl_->init_debug_adbroker( this );
            } catch ( std::exception& ex ) {
                adportable::debug(__FILE__, __LINE__) << ex.what();
            }
	    
        } else if ( it->name() == L"adcontroller" ) {
            try {
                pImpl_->init_debug_adcontroller( this );
            } catch ( std::exception& ex ) {
                adportable::debug(__FILE__, __LINE__) << ex.what();
                continue;
            }

            CORBA::Object_var obj 
                = acewrapper::brokerhelper::name_to_object( pMgr->orb()
                                                            , acewrapper::constants::adcontroller::manager::_name()
                                                            , adplugin::manager::iorBroker() ); 
            ControlServer::Manager_var manager = ControlServer::Manager::_narrow( obj );
            if ( CORBA::is_nil( manager ) ) {
                error_message = new QString("ControlServer::Manager object reference failed");
                return false;
            }
            session = manager->getSession( L"debug" );
            session->setConfiguration( it->xml().c_str() );

        } else if ( it->attribute( L"type" ) == L"orbLoader" ) {
	    
            if ( it->attribute( L"loadstatus" ) == L"failed" || it->attribute( L"loadstatus" ) == L"deffered" )
                continue;
            std::string ns_name = adportable::string::convert( it->attribute( L"ns_name" ) );
            if ( ! ns_name.empty() ) {
                CORBA::String_var ior = broker_manager_->ior( ns_name.c_str() );
                try {
                    CORBA::Object_var obj = adplugin::ORBManager::instance()->string_to_object( ior.in() );                     
                    Instrument::Session_var isession = Instrument::Session::_narrow( obj );
                    if ( ! CORBA::is_nil( isession ) ) {
                        isession->setConfiguration( it->xml().c_str() );
                        i8t_sessions.push_back( isession );
                    }
                } catch ( CORBA::Exception& src ) {
                    adportable::debug dbg(__FILE__, __LINE__ );
                    dbg << "CORBA::Exceptiron while referenceing '" << ns_name.c_str() << "' by " << src._info().c_str();
                    Logger log;
                    log( dbg.str() );
                    QMessageBox::critical( 0, "ServantPlugin", dbg.str().c_str() );
                    ++nErrors;
                }
            }
        } else if ( it->attribute( L"type" ) == L"MassSpectrometer" ) {
            // const std::wstring name = apppath + it->module().library_filename();
            const std::wstring name = adplugin::orbLoader::library_fullpath( apppath, it->module().library_filename() );
            adcontrols::MassSpectrometerBroker::register_library( name );
        }
    }
#endif

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
    if ( ! CORBA::is_nil( broker_manager_.in() ) ) // Qt5 does not call shutdown
        broker_manager_->shutdown();
	return SynchronousShutdown;
}

void
ServantPlugin::final_close()
{
    adportable::debug() << "====== ServantPlugin::final_close ... =======";
//    adportable::Configuration& config = *pConfig_;

    // destriction must be reverse order
    for ( orbservant_vector_type::reverse_iterator it = orbServants_.rbegin(); it != orbServants_.rend(); ++it )
        (*it)->deactivate();

    adportable::debug() << "====== ServantPlugin::final_close Loggor::shutdown... =======";    
    Logger::shutdown();
    try {
        adportable::debug() << "====== ServantPlugin::final_close orb shutdown... =======";    
		servant::ORBServantManager::instance()->shutdown();
        adportable::debug() << "====== ServantPlugin::final_close orb fini... =======";    
        servant::ORBServantManager::instance()->fini();
    } catch ( CORBA::Exception& ex ) {
		adportable::debug dbg( __FILE__, __LINE__ );
		dbg << ex._info().c_str();
        QMessageBox::critical( 0, dbg.where().c_str(), dbg.str().c_str() );
	}

    adportable::debug() << "====== ServantPlugin::final_close waiting threads... =======";    
    ACE_Thread_Manager::instance()->wait();
    adportable::debug() << "====== ServantPlugin::final_close complete =======";    
}


Q_EXPORT_PLUGIN( ServantPlugin )
