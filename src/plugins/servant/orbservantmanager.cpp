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

#include "orbservantmanager.hpp"
#include "servant.hpp"
#include <tao/Utils/ORB_Manager.h>
#include <adportable/debug.hpp>
#include <boost/bind.hpp>
#include <boost/thread/barrier.hpp>

using namespace servant;

ORBServantManager * ORBServantManager::instance_ = 0;

ORBServantManager *
ORBServantManager::instance()
{
    if ( instance_ == 0 ) {
        boost::mutex::scoped_lock lock( Servant::instance().mutex_ );
        if ( instance_ == 0 ) 
            instance_ = new ORBServantManager();
    }
    return instance_;
}

ORBServantManager::~ORBServantManager()
{
    delete orbmgr_;
    adportable::debug(__FILE__, __LINE__) << "ORBServantManager deleted cleanly";
}

ORBServantManager::ORBServantManager( CORBA::ORB_ptr orb
                                      , PortableServer::POA_ptr poa
                                      , PortableServer::POAManager_ptr poamanager ) : init_count_(0)  
                                                                                    , thread_running_(false)
                                                                                    , orbmgr_(0)
                                                                                    , thread_(0)
{
    orbmgr_ = new TAO_ORB_Manager( orb, poa, poamanager );
}

int
ORBServantManager::init( int ac, ACE_TCHAR * av[] )
{
    boost::mutex::scoped_lock lock( mutex_ );

    if ( init_count_++ == 0 )
        return orbmgr_->init( ac, av );

    return 0;
}

bool
ORBServantManager::fini()
{
    boost::mutex::scoped_lock lock( mutex_ );

    if ( init_count_ && --init_count_ == 0 )
        return orbmgr_->fini() == 0;

    return false;
}

bool
ORBServantManager::wait()
{
    if ( thread_ ) {
        thread_->join();
        return true;
    }
    return false;
}

boost::mutex&
ORBServantManager::mutex()
{
	return mutex_;
}

CORBA::ORB_ptr
ORBServantManager::orb()
{
    return orbmgr_->orb();
}

PortableServer::POA_ptr
ORBServantManager::root_poa()
{
    if ( orbmgr_ )
        return orbmgr_->root_poa();
    return 0;
}

PortableServer::POA_ptr
ORBServantManager::child_poa()
{
    if ( orbmgr_ )
        return orbmgr_->child_poa();
    return 0;
}

PortableServer::POAManager_ptr
ORBServantManager::poa_manager()
{
    if ( orbmgr_ )
        return orbmgr_->poa_manager();
    return 0;
}

std::string
ORBServantManager::activate( PortableServer::Servant servant )
{
    CORBA::String_var id = orbmgr_->activate( servant );
    return std::string ( id.in() );
}

void
ORBServantManager::deactivate( const std::string& id )
{
    orbmgr_->deactivate( id.c_str() );
}

void
ORBServantManager::shutdown()
{
    CORBA::ORB_var orb;

    if ( orbmgr_ && ( orb = orbmgr_->orb() ) ) {
		if ( ! CORBA::is_nil( orb ) )
			orb->shutdown( true );
	}
}

void
ORBServantManager::run( boost::barrier& barrier )
{
    try {
		barrier.wait();
        orbmgr_->run();
        thread_running_ = false;
    } catch ( ... ) {
        thread_running_ = false;
        throw;
    }
	adportable::debug(__FILE__, __LINE__) << "ORBServantManager::run -- terminated.";
}

bool
ORBServantManager::spawn( boost::barrier& barrier )
{
    if ( thread_ == 0 ) {
        boost::mutex::scoped_lock lock( mutex_ );
        if ( thread_ == 0 ) {
            thread_ = new boost::thread( boost::bind( &ORBServantManager::run, this, boost::ref(barrier) ) );
            thread_running_ = true;
            return true;
        }
    }
    return false;
}

