/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#pragma once

#include "tofmgr_i.hpp"
#include "tofsession_i.hpp"
#include <xmlparser/pugixml.hpp>
#include <acewrapper/constants.hpp>
#include <acewrapper/orbservant.hpp>
#include <adinterface/controlserverC.h>

using namespace tofservant;

tofmgr_i::tofmgr_i() : tofSession_( new acewrapper::ORBServant< tofSession_i > )
{
}

tofmgr_i::~tofmgr_i()
{
}

bool
tofmgr_i::setBrokerManager( Broker::Manager_ptr mgr )
{
	broker_mgr_ = Broker::Manager::_duplicate( mgr );
    if ( !CORBA::is_nil( broker_mgr_.in() ) && tofSession_ ) {
        // name should be match up with ns_name, which is described in tofservant.adplugin file
        // in this directory
		CORBA::Object_var obj = *tofSession_;
        broker_mgr_->register_object( "com.ms-cheminfo.qtplatz.instrument.session.tofservant", obj );
    }
    return true;
}

bool
tofmgr_i::adpluginspec( const char * id, const char * xml )
{
	adplugin_id_ = id;
	adplugin_spec_ = xml;

	pugi::xml_document dom;
	pugi::xml_parse_result result;

	if ( ( result = dom.load( xml ) ) ) {
        
        if ( ! CORBA::is_nil( broker_mgr_.in() ) ) {
            CORBA::Object_var obj = broker_mgr_->find_object( acewrapper::constants::adcontroller::manager::_name() );
            ControlServer::Manager_var ctrlMgr = ControlServer::Manager::_narrow( obj );
            if ( ! CORBA::is_nil( ctrlMgr.in() ) ) {
                ControlServer::Session_var session = ctrlMgr->getSession( L"tofmgr_i" );
                if ( ! CORBA::is_nil( session ) )
                    session->setConfiguration( xml );
            }
        }
    }
    return true;
}
