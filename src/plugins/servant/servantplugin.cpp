// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <extensionsystem/pluginmanager.h>
#include <QtCore/qplugin.h>
#include <QtCore>
#include <qdebug.h>
#include <boost/filesystem.hpp>


#if defined _MSC_VER
#  pragma warning(disable:4996)
#endif
# include <ace/Thread_Manager.h>
# include <ace/OS_NS_unistd.h>
#if defined _MSC_VER
#  pragma warning(default:4996)
#endif

#include <adbroker/adbroker.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontroller/adcontroller.hpp>
#include <adinterface/instrumentC.h>

#include <adplugin/adplugin.hpp>
#include <adplugin/orbLoader.hpp>
#include <adplugin/orbmanager.hpp>
#include <adplugin/constants.hpp>

#include <adportable/configuration.hpp>
#include <adportable/string.hpp>
#include <adportable/debug.hpp>
#include <acewrapper/acewrapper.hpp>
//#include <acewrapper/orbservant.h>
#include <acewrapper/constants.hpp>
#include <acewrapper/brokerhelper.hpp>
#include <qtwrapper/qstring.hpp>
#include "outputwindow.hpp"
#include "servantpluginimpl.hpp"
#include <QMessageBox>
#include <boost/format.hpp>
#include "orbservantmanager.hpp"

using namespace servant;
using namespace servant::internal;

ServantPlugin::~ServantPlugin()
{
    final_close();
    delete pImpl_;
    delete pConfig_;
}

ServantPlugin::ServantPlugin() : pConfig_( 0 )
                               , pImpl_( 0 )
{
}

namespace servant {
    namespace internal {
        class ServantUtils {
        public:
#if 0
            static Broker::Manager_ptr initialize_broker( CORBA::ORB_ptr orb
                                                          , PortableServer::POA_ptr poa
                                                          , PortableServer::POAManager_ptr poaMgr
                                                          , std::string& iorBroker )	{
                adBroker::initialize( orb, poa, poaMgr ); 
                iorBroker = adBroker::activate();
                // TODO: check if iorBroker is approprate or not
                if ( ! iorBroker.empty() )
                    adplugin::manager::instance()->register_ior( acewrapper::constants::adbroker::manager::_name(), iorBroker ;)
                
                // ---- private naming service (Broker::Manager) start ----
                CORBA::Object_var obj = orb->string_to_object( iorBroker.c_str() );
                Broker::Manager_var mgr = Broker::Manager::_narrow( obj );
                if ( CORBA::is_nil( mgr ) )
                    return 0;
                // store broker's ior to self
                for ( size_t nTrial = 0; nTrial < 25 ; ++nTrial ) {
                    try {
                        mgr->register_ior( acewrapper::constants::adbroker::manager::_name(), iorBroker.c_str() );
                        break;
                    } catch ( CORBA::Exception& ex ) {
                        adportable::debug( __FILE__, __LINE__ ) << "CORBA::Exception: " << ex._info().c_str();
                        if ( nTrial > 5 )
                            QMessageBox::warning(0, "ServantPlugin - Broker::Manager::registor_ior", ex._info().c_str() );
                        else
                            QMessageBox::critical(0, "ServantPlugin - Broker::Manager::registor_ior", ex._info().c_str() );
                        ACE_OS::sleep(0);
                    }
                }
                return mgr._retn();
            }
#endif
        };
        struct adbroker_initializer {
            servant::ORBServantManager * pMgr_;
            std::wstring file_;
            std::string ior_;

            adbroker_initializer( servant::ORBServantManager * p, const std::wstring& file )
                : pMgr_(p), file_(file) {
            }

            bool operator()( adportable::Configuration::vector_type::iterator it ) {
                adplugin::orbLoader& loader = adplugin::manager::instance()->orbLoader( file_ );
                if ( loader ) {
                    loader.initialize( pMgr_->orb(), pMgr_->root_poa(), pMgr_->poa_manager() );
                    ior_ = loader.activate();
                    if ( ! ior_.empty() )
                        adplugin::manager::instance()->register_ior( acewrapper::constants::adbroker::manager::_name(), ior_ );
                    return true;
                } else {
                    it->attribute( L"loadstatus", L"failed" );
                    return false;
                }
            }

            Broker::Manager_ptr getObject() {
                // ---- private naming service (Broker::Manager) start ----
                CORBA::Object_var obj = pMgr_->orb()->string_to_object( ior_.c_str() );
                Broker::Manager_var mgr = Broker::Manager::_narrow( obj );
                if ( CORBA::is_nil( mgr ) )
                    return 0;
                // store broker's ior to self
                for ( size_t nTrial = 0; nTrial < 25 ; ++nTrial ) {
                    try {
                        mgr->register_ior( acewrapper::constants::adbroker::manager::_name(), ior_.c_str() );
                        break;
                    } catch ( CORBA::Exception& ex ) {
                        adportable::debug( __FILE__, __LINE__ ) << "CORBA::Exception: " << ex._info().c_str();
                        if ( nTrial > 5 )
                            QMessageBox::warning(0, "ServantPlugin - Broker::Manager::registor_ior", ex._info().c_str() );
                        else
                            QMessageBox::critical(0, "ServantPlugin - Broker::Manager::registor_ior", ex._info().c_str() );
                        ACE_OS::sleep(0);
                    }
                }
                return mgr._retn();
            }
        };


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
    
    OutputWindow * outputWindow = new OutputWindow;
    addAutoReleasedObject( outputWindow );
    pImpl_ = new internal::ServantPluginImpl( outputWindow );
    
    // ACE initialize
    acewrapper::instance_manager::initialize();
    // <------
    
    std::wstring apppath, configFile;

    do {
        QDir dir = QCoreApplication::instance()->applicationDirPath();
        dir.cdUp();
        apppath = qtwrapper::wstring::copy( dir.path() );
        configFile = adplugin::orbLoader::config_fullpath( apppath, L"/ScienceLiaison/servant.config.xml" );
    } while(0);
    
    const wchar_t * query = L"/ServantConfiguration/Configuration";
    
    pConfig_ = new adportable::Configuration();
    adportable::Configuration& config = *pConfig_;

    adportable::debug( __FILE__, __LINE__ ) << "loading configuration:" << configFile;

    if ( ! adplugin::manager::instance()->loadConfig( config, configFile, query ) ) {
		adportable::debug dbg( __FILE__, __LINE__ );
		dbg << "ServantPlugin::initialize loadConfig '" << configFile << "' load failed";
		QMessageBox::warning( 0, dbg.where().c_str(), dbg.str().c_str() );
    }
    
    // ------------ Broker::Manager initialize first --------------------
    servant::ORBServantManager * pMgr = servant::singleton::orbServantManager::instance();
    pMgr->init( 0, 0 );
    if ( pMgr->test_and_set_thread_flag() ) {
        ACE_Thread_Manager::instance()->spawn( ACE_THR_FUNC( servant::ORBServantManager::thread_entry ), reinterpret_cast<void *>(pMgr) );
        ACE_OS::sleep(0);
    }
    adplugin::ORBManager::instance()->initialize( pMgr->orb(), pMgr->root_poa() );
    
    //--------------------------------------------------------------------
    //CORBA::ORB_var orb = acewrapper::singleton::orbServantManager::instance()->orb();
    std::string iorBroker;
    Broker::Manager_var mgr;
    do {
        internal::config_finder finder( config );
        adportable::Configuration::vector_type::iterator it = finder( L"adbroker" );
        if ( it != config.end() ) {
            std::wstring file = adplugin::orbLoader::library_fullpath( apppath, it->module().library_filename() );
            it->attribute( L"fullpath", file );
            internal::adbroker_initializer initializer( pMgr, file );
            if ( initializer( it ) ) {
                iorBroker = initializer.ior_;
                mgr = initializer.getObject();
            }
        }
        if ( CORBA::is_nil( mgr ) || iorBroker.empty() ) {
            QMessageBox::critical(0, "ServantPlugin", "Broker::Manager creation failed" );
            *error_message = "Broker::Manager creation failed";
            return false;
        }
    } while(0);

    ///////////////////////////////////
    for ( adportable::Configuration::vector_type::iterator it = config.begin(); it != config.end(); ++it ) {
	std::wstring name = it->name();
	std::wstring component = it->component();
        
	if ( name == L"adbroker" ) {
	    /* nothing, broker must be created before here */
	} else if ( it->attribute(L"type") == L"orbLoader" ) {
	    // adcontroller must be on top
	    std::wstring file = adplugin::orbLoader::library_fullpath( apppath, it->module().library_filename() );
	    it->attribute( L"fullpath", file );
	    std::string ns_name = adportable::string::convert( it->attribute( L"ns_name" ) );
	    adportable::debug(__FILE__, __LINE__) << "orbLoader file=" << file << "\tns_name=" << ns_name;

	    adplugin::orbLoader& loader = adplugin::manager::instance()->orbLoader( file );
	    if ( loader ) {
			loader.initialize( pMgr->orb(), pMgr->root_poa(), pMgr->poa_manager() );
			loader.initial_reference( iorBroker.c_str() );
			std::string ior = loader.activate();            // activate object
			mgr->register_ior( ns_name.c_str(), ior.c_str() ); // set ior to Broker::Manager
	    } else {
			it->attribute( L"loadstatus", L"failed" );
			// QMessageBox::warning( 0, "ServantPlugin", loader.error_description() );
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
	    
            if ( it->attribute( L"loadstatus" ) == L"failed" )
                continue;
	    std::string ns_name = adportable::string::convert( it->attribute( L"ns_name" ) );
	    if ( ! ns_name.empty() ) {
                CORBA::String_var ior = mgr->ior( ns_name.c_str() );
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
		    QMessageBox::critical( 0, "Servant error", dbg.str().c_str() );
		    ++nErrors;
		}
	    }
	} else if ( it->attribute( L"type" ) == L"MassSpectrometer" ) {
	    const std::wstring name = apppath + it->module().library_filename();
	    adcontrols::MassSpectrometerBroker::register_library( name );
	}
    }
    
    if ( ! CORBA::is_nil( session ) ) {
	for ( std::vector< Instrument::Session_var >::iterator it = i8t_sessions.begin(); it != i8t_sessions.end(); ++it )
	    (*it)->configComplete();
	session->configComplete();
    }
    Logger log;
    log( ( nErrors ? L"Servant iitialized with errors" : L"Servernt initialized successfully") );
    
    do { adportable::debug() << "ServantPlugin::initialize() completed."; } while(0);
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

void
ServantPlugin::shutdown()
{
}

void
ServantPlugin::final_close()
{
    adportable::Configuration& config = *pConfig_;
    
    // destriction must be reverse order
    for ( adportable::Configuration::vector_type::reverse_iterator it = config.rbegin(); it != config.rend(); ++it ) {
        if ( it->attribute(L"type") == L"orbLoader" ) {
            std::wstring file = it->attribute( L"fullpath" );
            adplugin::orbLoader& loader = adplugin::manager::instance()->orbLoader( file );
            if ( loader )
                loader.deactivate();
        }
    }
    Logger::shutdown();
    /***
    // now adBroker is in list of above vector, should be deactivated by now
    try {
        // adBroker::deactivate();
    } catch ( CORBA::Exception& ex ) {
        adportable::debug dbg( __FILE__, __LINE__ );
        dbg << ex._info().c_str();
        QMessageBox::critical( 0, dbg.where().c_str(), dbg.str().c_str() );
    }
    ***/
    try {
        servant::singleton::orbServantManager::instance()->orb()->shutdown();
        servant::singleton::orbServantManager::instance()->fini();
    } catch ( CORBA::Exception& ex ) {
        adportable::debug dbg( __FILE__, __LINE__ );
        dbg << ex._info().c_str();        dbg << ex._info().c_str();
        QMessageBox::critical( 0, dbg.where().c_str(), dbg.str().c_str() );
    }
    ACE_Thread_Manager::instance()->wait();
}


Q_EXPORT_PLUGIN( ServantPlugin )
