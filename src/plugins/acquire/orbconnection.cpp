// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "orbconnection.hpp"

#include <adinterface/brokerC.h>
#include <adinterface/brokerclientC.h>
#include <adorbmgr/orbmgr.hpp>
#include <adplugin/orbfactory.hpp>
#include <adplugin/loader.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adplugin/orbbroker.hpp>
#include <adplugin/orbservant.hpp>
#include <tao/Object.h>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <QMessageBox>

namespace acquire {
    std::atomic< OrbConnection * > OrbConnection::instance_(0);
    std::mutex OrbConnection::mutex_;
}

using namespace acquire;

OrbConnection::~OrbConnection()
{
}

OrbConnection::OrbConnection() : initialized_( false )
{
}

OrbConnection *
OrbConnection::instance()
{
    typedef OrbConnection T;

    T * tmp = instance_.load( std::memory_order_relaxed );
    std::atomic_thread_fence( std::memory_order_acquire );
    if ( tmp == nullptr ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        tmp = instance_.load( std::memory_order_relaxed );
        if ( tmp == nullptr ) {
            tmp = new T();
            std::atomic_thread_fence( std::memory_order_release );
            instance_.store( tmp, std::memory_order_relaxed );
        }
    }
    return tmp;
}

bool
OrbConnection::initialize()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    
    if ( initialized_ )
        return false;

    auto mgr = adorbmgr::orbmgr::instance();
    adplugin::orbServant * adBroker = 0;
    adplugin::orbBroker * orbBroker = 0;
    
    adplugin::plugin_ptr adbroker_plugin = adplugin::loader::select_iid( ".*\\.orbfactory\\.adbroker" );
    if ( adbroker_plugin ) {
        
        if ( (orbBroker = adbroker_plugin->query_interface< adplugin::orbBroker >()) ) {
            
            try {
                orbBroker->orbmgr_init( 0, 0 );
            } catch ( CORBA::Exception& ex ) {
                ADERROR() << "orbBroker raize an exception: " << ex._info().c_str();
                QMessageBox::warning( 0, "AcquirePlugin", QString::fromStdString( boost::current_exception_diagnostic_information() ) );
                return false;
            } catch ( ... ) {
                ADERROR() << "orbBroker raize an exception: " << boost::current_exception_diagnostic_information();
                QMessageBox::warning( 0, "AcquirePlugin", QString::fromStdString( boost::current_exception_diagnostic_information() ) );
                return false;
            }

            try {
                if ( (adBroker = orbBroker->create_instance()) ) {
                    
                    adBroker->initialize( mgr->orb(), mgr->root_poa(), mgr->poa_manager() );
                    std::string ior = adBroker->activate();
                    orbServants_.push_back( adBroker );
                    //Broker::Manager_var mgr = Broker::Manager::_narrow( adBroker->_this() );
                    //addObject( new QBroker( mgr._retn() ) );
                }
            } catch ( CORBA::Exception& ex ) {
                ADERROR() << "orbBroker raize an exception: " << ex._info().c_str();
            } catch ( ... ) {
                ADERROR() << "orbBroker raize an exception: " << boost::current_exception_diagnostic_information();
                QMessageBox::warning( 0, "AcquirePlugin", QString::fromStdString( boost::current_exception_diagnostic_information() ) );
                return false;
            }
        }
    }
    if ( adBroker == 0 || orbBroker == 0 ) {
        ADTRACE() << "adBroker does not initialized (it might be not configured)";
        return false;
    }
    Broker::Manager_var bMgr( Broker::Manager::_narrow( adBroker->_this() ) );
    if ( CORBA::is_nil( bMgr ) )
        return false;

    // ----------------------- initialize corba servants ------------------------------
    std::vector< adplugin::plugin_ptr > factories;
    adplugin::loader::select_iids( ".*\\.adplugins\\.orbfactory\\..*", factories );
    for ( const adplugin::plugin_ptr& plugin : factories ) {
        
        if ( plugin->iid() == adbroker_plugin->iid() ) // skip "adBroker"
            continue;
        
        std::string clsid = plugin->clsid();
        ADTRACE() << "initializing " << clsid << "{iid: " << plugin->iid() << "}";
        if ( auto factory = plugin->query_interface< adplugin::orbFactory >() ) {
            if ( auto servant = factory->create_instance() ) {
                try {
                    servant->initialize( mgr->orb(), mgr->root_poa(), mgr->poa_manager() );
                    servant->activate();

                    CORBA::Object_var obj( servant->_this() );
                    if ( !CORBA::is_nil( obj.in() ) ) {

                        BrokerClient::Accessor_var accessor( BrokerClient::Accessor::_narrow( obj ) );
                        if ( !CORBA::is_nil( accessor.in() ) ) {

                            accessor->setBrokerManager( bMgr );
                            accessor->adpluginspec( plugin->clsid(), plugin->adpluginspec() );

                            try {
                                bMgr->register_object( servant->object_name(), obj );
                                orbServants_.push_back( servant );

                            } catch ( CORBA::Exception& ex ) {
                                factory->release( servant );
                                ADERROR() << ex._info().c_str();
                            }
                        }
                    } else {
                        factory->release( servant );
                    }
                } catch ( ... ) {
                    ADERROR() << "exception while initializing " << plugin->clsid() << "\t" << boost::current_exception_diagnostic_information();
                    QMessageBox::warning( 0, "Exception AcquirePlugin"
                                          , "If you are on Windows 7 sp1, some of important Windows update is getting involved. \
                                            Please make sure you have up-to-date for Windows" );
                }
            }

        }
    }
    initialized_ = true;
    return true;
}

Broker::Manager_ptr
OrbConnection::brokerManager()
{
    if ( !orbServants_.empty() ) {
        if ( auto servant = orbServants_[ 0 ] ) {  // 1'st item is Broker::Manager
            return Broker::Manager::_narrow( servant->_this() );
        }
    }
    return 0;
}


void
OrbConnection::shutdown()
{
    ADTRACE() << "OrbConnection::shutdown_broker() ...";

    // auto iBroker = ExtensionSystem::PluginManager::instance()->getObject< adextension::iBroker >();
	// removeObject( iBroker );

    // destriction must be reverse order
    for ( orbservant_vector_type::reverse_iterator it = orbServants_.rbegin(); it != orbServants_.rend(); ++it )
        (*it)->deactivate();

	if ( adplugin::plugin_ptr adbroker_plugin = adplugin::loader::select_iid( ".*\\.orbfactory\\.adbroker" ) ) {

        if ( adplugin::orbBroker * orbBroker = adbroker_plugin->query_interface< adplugin::orbBroker >() ) {
            
            try {

                orbBroker->orbmgr_shutdown();
                orbBroker->orbmgr_fini();
                orbBroker->orbmgr_wait();

            } catch ( boost::exception& ex ) {
                ADERROR() << boost::diagnostic_information( ex );
            }
            
        }
    }
    ADTRACE() << "OrbConnection::shutdown_broker() completed.";
}

