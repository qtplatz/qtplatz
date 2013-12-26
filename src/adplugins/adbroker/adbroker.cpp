// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "adbroker.hpp"
#include "orbbroker.hpp"
#include <adplugin/visitor.hpp>
#include <adportable/debug.hpp>
#include <typeinfo>
#include <tao/Utils/ORB_Manager.h>
#include <acewrapper/orbservant.hpp>
#include <acewrapper/constants.hpp>
#include <adportable/debug.hpp>
#include "manager_i.hpp"
#include "brokermanager.hpp"
#include <mutex>

using namespace acewrapper;

adBroker::adBroker(void)
{
}

adBroker::~adBroker(void)
{
}

void *
adBroker::query_interface_workaround( const char * _typenam )
{
	const std::string typenam( _typenam );

    if ( typenam == typeid( adplugin::orbServant ).name() )
        return static_cast< adplugin::orbServant * >(this);
    else if ( typenam == typeid( adplugin::plugin ).name() )
        return static_cast< adplugin::plugin * >(this);
    return 0;
}

bool
adBroker::initialize( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, PortableServer::POAManager_ptr mgr )
{
    adbroker::manager_i::instance()->initialize( orb, poa, mgr );
	return true;
}

const char *
adBroker::activate()
{
	adbroker::manager_i::instance()->activate();
	return adbroker::manager_i::instance()->ior().c_str();
}

bool
adBroker::deactivate()
{
	adbroker::BrokerManager::terminate(); // terminate task
	adbroker::manager_i::instance()->deactivate();
	return true;
}

const char *
adBroker::object_name() const
{
	return acewrapper::constants::adbroker::manager::_name();
}

void
adBroker::initial_reference( const char * )
{
    // do nothing
}


class adbroker_plugin : public adplugin::plugin
                      , public adplugin::orbFactory
                      , public adbroker::orbBroker {

    static adbroker_plugin * instance_;
    adbroker_plugin() {}
    ~adbroker_plugin() {}
public:
    static adbroker_plugin * instance();
    // plugin
    virtual const char * iid() const;
    virtual void accept( adplugin::visitor&, const char * );
    virtual void * query_interface_workaround( const char * typenam );

    // orbFactory
    virtual adplugin::orbServant * create_instance() {
        return new adBroker;        
    }
};

adbroker_plugin * adbroker_plugin::instance_ = 0;
static std::mutex __mutex;

adbroker_plugin *
adbroker_plugin::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( __mutex );
        if ( instance_ == 0 )
            instance_ = new adbroker_plugin();
    }
    return instance_;
}

const char *
adbroker_plugin::iid() const
{
    return "com.ms-cheminfo.qtplatz.adplugins.orbfactory.adbroker";
}

void
adbroker_plugin::accept( adplugin::visitor& v, const char * adplugin )
{
	v.visit( this, adplugin );
}

void *
adbroker_plugin::query_interface_workaround( const char * typenam )
{
    if ( std::string( typenam ) == typeid( orbFactory ).name() )
        return static_cast<orbFactory *>(this);
    else if ( std::string( typenam ) == typeid( adbroker::orbBroker ).name() )
        return static_cast<orbFactory *>(this);
    return 0;
}

Q_DECL_EXPORT adplugin::plugin * adplugin_plugin_instance()
{
    return adbroker_plugin::instance();
}
