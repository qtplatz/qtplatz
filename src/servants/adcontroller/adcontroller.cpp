/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "adcontroller.hpp"
#include <adplugin/plugin.hpp>
#include <adplugin/visitor.hpp>
#include <adplugin/orbfactory.hpp>
#include <adplugin/orbservant.hpp>
#include <boost/thread/mutex.hpp>
#include "ace/Init_ACE.h"

# include <ace/SOCK_Dgram_Mcast.h>
# include <ace/Service_Config.h>
# include <ace/Sched_Params.h>
# include <ace/Thread_Manager.h>
# include <ace/Process_Manager.h>
# include <tao/Utils/ORB_Manager.h>

#include "signal_handler.hpp"
#include <signal.h>
#include <acewrapper/orbservant.hpp>

#include <iostream>
#include <fstream>

#include <acewrapper/constants.hpp>
#include <boost/smart_ptr.hpp>
#include "manager_i.hpp"
#include "taskmanager.hpp"

using namespace acewrapper;

static int debug_flag = 0;
static bool __aborted = false;
std::string __ior_session;

boost::mutex __mutex;

//--------------------
class adcontroller_plugin : public adplugin::plugin 
                          , public adplugin::orbFactory {
    static adcontroller_plugin * instance_;
public:
    static inline adcontroller_plugin *instance() { 
        if ( instance_ == 0 ) {
            boost::mutex::scoped_lock lock( __mutex );
            if ( instance_ == 0 )
                instance_ = new adcontroller_plugin();
        }
        return instance_;
    }
    // adplugin::plugin
    virtual const char * iid() const;
    virtual void accept( adplugin::visitor&, const char * );
    virtual void * query_interface_workaround( const char * typenam );
    virtual adplugin::orbServant * create_instance();
};

adcontroller_plugin * adcontroller_plugin::instance_ = 0;

//-----------------------------------------------

adController::~adController()
{
}

adController::adController()
{
}

void
adController::_abort_server()
{
    __aborted = true;
    adController::_deactivate();
}

bool
adController::initialize( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, PortableServer::POAManager_ptr mgr )
{
    ORBServant< adcontroller::manager_i > * pServant = adcontroller::manager_i::instance();
    pServant->initialize( orb, poa, mgr );
    return true;
}

const char *
adController::activate()
{
    ORBServant< adcontroller::manager_i > * pServant = adcontroller::manager_i::instance();
    pServant->activate();
    return pServant->ior().c_str();
}

bool
adController::deactivate()
{
    return adController::_deactivate();
}

const char *
adController::object_name() const
{
	return acewrapper::constants::adcontroller::manager::_name();
}

bool
adController::_deactivate()
{
    adcontroller::iTaskManager::instance()->manager_terminate();
    adcontroller::manager_i::instance()->deactivate();
    return true;
}

int
adController::run()
{
    ACE_Sched_Params fifo_sched_params( ACE_SCHED_FIFO, 
					ACE_Sched_Params::priority_min( ACE_SCHED_FIFO ),
					ACE_SCOPE_PROCESS );
    
    //<---- set real time priority class for this process
    if ( debug_flag == 0 ) {
	if ( ACE_OS::sched_params(fifo_sched_params) == -1 ) {
	    if ( errno == EPERM || errno == ENOTSUP ) 
		ACE_DEBUG((LM_DEBUG, "Warning: user's not superuser, so we'll run in the theme-shared class\n"));
	    else
		ACE_ERROR_RETURN((LM_ERROR, "%p\n", "ACE_OS::sched_params()"), -1);
	}
    } else {
	std::cerr << "==================================================" << std::endl;
	std::cerr << "====== running normal priority for debug =========" << std::endl;
	std::cerr << "==================================================" << std::endl;
    }
    //-------------> end priority code
    return 0;
}

void
adController::abort_server()
{
    adController::_abort_server();
}


const char *
adcontroller_plugin::iid() const
{
    return "com.ms-cheminfo.qtplatz.plugin.orbfactory.adcontroller";
}

void
adcontroller_plugin::accept( adplugin::visitor& v, const char * adpluginspec )
{
    v.visit( this, adpluginspec );
}

void * 
adcontroller_plugin::query_interface_workaround( const char * typenam )
{
    if ( std::string( typenam ) == typeid( orbFactory ).name() )
        return static_cast< orbFactory * >(this);
    return 0;
}

adplugin::orbServant *
adcontroller_plugin::create_instance()
{
    return new adController();
}

/////////////////////

Q_DECL_EXPORT adplugin::plugin * adplugin_plugin_instance()
{
    return adcontroller_plugin::instance();
}

