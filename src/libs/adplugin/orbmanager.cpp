// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "orbmanager.hpp"
#include <assert.h>
#include <acewrapper/mutex.hpp>
#include <adportable/debug.hpp>

# include <tao/Utils/ORB_Manager.h>

using namespace adplugin;

ORBManager::~ORBManager()
{
	acewrapper::scoped_mutex_t<> lock( mutex_ );
	//delete orb_;
}

ORBManager::ORBManager() : orb_(0)
{
}

void
ORBManager::initialize( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa )
{
	orb_ = CORBA::ORB::_duplicate( orb );
    poa_ = PortableServer::POA::_duplicate( poa );
}

CORBA::ORB_ptr
ORBManager::orb()
{
	return CORBA::ORB::_duplicate( orb_.in() );
}

PortableServer::POA_ptr
ORBManager::poa()
{
    return PortableServer::POA::_duplicate( poa_.in() );
}

bool
ORBManager::deactivate( CORBA::Object_ptr obj )
{
    try {
        PortableServer::ObjectId_var object_id = poa_->reference_to_id( obj );
        poa_->deactivate_object( object_id );
    } catch ( CORBA::Exception& ex ) {
        adportable::debug( __FILE__, __LINE__ ) << ex._info().c_str();
        return false;
    }
    return true;
}

bool
ORBManager::deactivate( PortableServer::ServantBase * p_servant )
{
    try {
        PortableServer::ObjectId_var object_id = poa_->servant_to_id( p_servant );
        poa_->deactivate_object( object_id );
    } catch ( CORBA::Exception& ex ) {
        adportable::debug( __FILE__, __LINE__ ) << ex._info().c_str();
        return false;
    }
    return true;
}

CORBA::Object_ptr
ORBManager::string_to_object( const std::string& ior )
{
	if ( CORBA::is_nil( orb_ ) )
	  return 0;
	return orb_->string_to_object( ior.c_str() );
}

adplugin::ORBManager * 
ORBManager::instance()
{
    typedef ACE_Singleton< adplugin::ORBManager, ACE_Recursive_Thread_Mutex > impl;
    return impl::instance();
}
