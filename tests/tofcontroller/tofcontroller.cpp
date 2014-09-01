// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "tofcontroller.h"
#include "tofsession_i.h"
#include <acewrapper/orbservant.h>  // servant template
#include <acewrapper/constants.h>

static bool __aborted = false;
static bool __own_thread = false;

#  if defined _DEBUG
#     pragma comment(lib, "TAO_Utilsd.lib")
#     pragma comment(lib, "TAO_PId.lib")
#     pragma comment(lib, "TAO_PortableServerd.lib")
#     pragma comment(lib, "TAO_AnyTypeCoded.lib")
#     pragma comment(lib, "TAOd.lib")
#     pragma comment(lib, "ACEd.lib")
#     pragma comment(lib, "adinterfaced.lib")
#     pragma comment(lib, "adportabled.lib")
#     pragma comment(lib, "acewrapperd.lib")
#     pragma comment(lib, "adplugind.lib")
#  else
#     pragma comment(lib, "TAO_Utils.lib")
#     pragma comment(lib, "TAO_PI.lib")
#     pragma comment(lib, "TAO_PortableServer.lib")
#     pragma comment(lib, "TAO_AnyTypeCode.lib")
#     pragma comment(lib, "TAO.lib")
#     pragma comment(lib, "ACE.lib")
#     pragma comment(lib, "adinterface.lib")
#     pragma comment(lib, "adportable.lib")
#     pragma comment(lib, "acewrapper.lib")
#     pragma comment(lib, "adplugin.lib")
#  endif

TofController::~TofController()
{
}

TofController::TofController()
{
}

TofController::operator bool () const
{ 
	return true;
}

bool
TofController::initialize( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, PortableServer::POAManager_ptr mgr)
{
	acewrapper::ORBServant< tofcontroller::tofSession_i >
		* pServant = tofcontroller::singleton::tofSession_i::instance();
    pServant->initialize( orb, poa, mgr );
	return true;
}

void
TofController::initial_reference( const char * ior_broker_manager )
{
    using namespace tofcontroller;
	singleton::tofSession_i::instance()->broker_manager_ior( ior_broker_manager );
}

const char *
TofController::activate()
{
	acewrapper::ORBServant< tofcontroller::tofSession_i > 
		* pServant = tofcontroller::singleton::tofSession_i::instance();
	pServant->activate();
	return pServant->ior().c_str();
}

bool
TofController::deactivate()
{
	tofcontroller::singleton::tofSession_i::instance()->deactivate();
	return true;
}

adplugin::orbLoader * instance()
{
	return new TofController();
}

