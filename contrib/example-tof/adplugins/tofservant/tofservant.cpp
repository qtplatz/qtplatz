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

#include "tofservant.hpp"
#include "tofmgr_i.hpp"
#include <adplugin/visitor.hpp>
#include <adportable/debug.hpp>
#include <typeinfo>

# include <tao/Utils/ORB_Manager.h>
# include <ace/Thread_Manager.h>
# include <ace/Process_Manager.h>

#include <acewrapper/orbservant.hpp>
#include <acewrapper/constants.hpp>
#include <acewrapper/acewrapper.hpp>
#include <adportable/debug.hpp>

#include <mutex>

using namespace tofservant;

tofServantPlugin * tofServantPlugin::instance_ = 0;
static std::mutex __mutex;

tofServantPlugin *
tofServantPlugin::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( __mutex );
        if ( instance_ == 0 )
            instance_ = new tofServantPlugin();
    }
    return instance_;
}

tofServantPlugin::tofServantPlugin() : tofMgr_( new acewrapper::ORBServant< tofmgr_i >() )
{
}

tofServantPlugin::~tofServantPlugin()
{
}

const char *
tofServantPlugin::iid() const
{
    return "com.ms-cheminfo.qtplatz.adplugins.orbfactory.tofservant";
}

void
tofServantPlugin::accept( adplugin::visitor& v, const char * adplugin )
{
	v.visit( this, adplugin );
}

void *
tofServantPlugin::query_interface_workaround( const char * typenam )
{
    if ( std::string( typenam ) == typeid( orbFactory ).name() )
        return static_cast<orbFactory *>(this);
    return 0;
}

// adplugin::orbServant
bool
tofServantPlugin::initialize( CORBA::ORB * orb
                              , PortableServer::POA * poa
							  , PortableServer::POAManager * mgr )
{
	tofMgr_->initialize( orb, poa, mgr );
	return true;
}

const char *
tofServantPlugin::activate()
{
    tofMgr_->activate();
    return tofMgr_->ior().c_str();
}

bool
tofServantPlugin::deactivate()
{
	// todo: stop internal threads
	tofMgr_->deactivate();
	return true;
}

const char *
tofServantPlugin::object_name() const
{
	// object_name is for find this object from related other objects such as UI module by name.
	return iid();
}

DECL_EXPORT adplugin::plugin * 
adplugin_plugin_instance()
{
    return tofServantPlugin::instance();
}
