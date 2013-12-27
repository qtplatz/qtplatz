/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "orbbroker.hpp"
#include "adbroker.hpp"
#include "manager_i.hpp"
#include <acewrapper/orbservant.hpp>
#include <adplugin/orbservant.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adplugin/orbfactory.hpp>
#include <adorbmgr/orbmgr.hpp>
#include <adportable/debug.hpp>
#include <adinterface/brokerC.h>
#include <adinterface/brokerclientC.h>
#include <boost/exception/all.hpp>


using namespace adbroker;

orbBroker::~orbBroker()
{
}

orbBroker::orbBroker()
{
}

bool
orbBroker::orbmgr_init( int ac, char * av [] ) const
{
    if ( adorbmgr::orbmgr * pMgr = adorbmgr::orbmgr::instance() ) {
        pMgr->init( ac, av );
        pMgr->spawn();
		return true;
    }
	return false;
}

void
orbBroker::orbmgr_shutdown()
{
    return adorbmgr::orbmgr::instance()->shutdown();
}

bool
orbBroker::orbmgr_fini()
{
    return adorbmgr::orbmgr::instance()->fini();
}

bool
orbBroker::orbmgr_wait()
{
    return adorbmgr::orbmgr::instance()->wait();
}

adplugin::orbServant *
orbBroker::create_instance() const
{
    if ( adorbmgr::orbmgr * pMgr = adorbmgr::orbmgr::instance() ) {

        if ( adBroker * broker = new adBroker ) {

            broker->initialize( pMgr->orb(), pMgr->root_poa(), pMgr->poa_manager() );
            std::string ior = broker->activate();

            if ( !ior.empty() ) {
                CORBA::Object_var obj = pMgr->orb()->string_to_object( ior.c_str() );
                Broker::Manager_var mgr = Broker::Manager::_narrow( obj );

                if ( !CORBA::is_nil( mgr ) ) {
                    adorbmgr::orbmgr::instance()->setBrokerManager( mgr );
                    size_t nTrial = 30;
                    while ( nTrial-- ) {
                        try {
                            mgr->register_ior( broker->object_name(), ior.c_str() );
                        } catch ( CORBA::Exception& ex ) {
                            if ( nTrial ) {
                                std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
                            } else {
								delete broker;
								broker = 0;
                                struct corba_error : virtual boost::exception, virtual std::exception {};
								typedef boost::error_info< struct tag_errmsg, std::string > info;
                                BOOST_THROW_EXCEPTION( corba_error() << info( ex._info().c_str() ) );
                            }
                        }
                    }
                }
            }
			return broker;
        }
    }
    return 0;
}


adplugin::orbServant *
orbBroker::operator()( adplugin::plugin * plugin ) const
{
    if ( plugin ) {
    
        adorbmgr::orbmgr * pMgr = adorbmgr::orbmgr::instance();
        if ( adplugin::orbFactory * factory = plugin->query_interface< adplugin::orbFactory >() ) {
            
            if ( adplugin::orbServant * orbServant = factory->create_instance() ) {
                
                orbServant->initialize( pMgr->orb(), pMgr->root_poa(), pMgr->poa_manager() );
                std::string ior = orbServant->activate();
                if ( !ior.empty() ) {
                    CORBA::Object_var obj = pMgr->orb()->string_to_object( ior.c_str() );
                    BrokerClient::Accessor_var accessor = BrokerClient::Accessor::_narrow( obj );

                    if ( !CORBA::is_nil( accessor ) ) {

						Broker::Manager_var mgr = adorbmgr::orbmgr::getBrokerManager();
                        
                        accessor->setBrokerManager( mgr.in() );
                        accessor->adpluginspec( plugin->clsid(), plugin->adpluginspec() );

                        try {
                            mgr->register_ior( orbServant->object_name(), ior.c_str() );
                            mgr->register_object( orbServant->object_name(), obj );
                        } catch ( CORBA::Exception& ex ) {
                            factory->release( orbServant );
                            struct corba_error : virtual boost::exception, virtual std::exception {};
                            typedef boost::error_info< struct tag_errmsg, std::string > info;
                            BOOST_THROW_EXCEPTION( corba_error() << info( ex._info().c_str() ) );
                        }
                    }
                    return orbServant;
                }
                
            } else {
                ADDEBUG() << plugin->clsid() << " nil instance created.";
            }
            
        } else {
            ADDEBUG() << plugin->clsid() << " has on adplugin::orbFactory class.";
        }
    }

    return 0;
}

