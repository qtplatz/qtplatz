/**************************************************************************
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#include "orbmgr.hpp"

#include <tao/Utils/ORB_Manager.h>
#include <adlog/logger.hpp>
#include <boost/exception/all.hpp>
#include <thread>

using namespace adorbmgr;

std::atomic<orbmgr * >orbmgr::instance_(0);
std::mutex orbmgr::mutex_;

orbmgr *
orbmgr::instance()
{
    typedef orbmgr T;

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

orbmgr::~orbmgr()
{
    delete taomgr_;
    ADTRACE() << "orbmgr deleted cleanly";
}

orbmgr::orbmgr( CORBA::ORB_ptr orb
                , PortableServer::POA_ptr poa
                , PortableServer::POAManager_ptr poamanager ) : thread_running_( false )
                                                              , init_count_( 0 )  
                                                              , thread_( 0 )
                                                              , taomgr_( new TAO_ORB_Manager( orb, poa, poamanager ) )
{
}

int
orbmgr::init( int ac, ACE_TCHAR * av[] )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( init_count_++ == 0 )
        return taomgr_->init( ac, av );

    return 0;
}

bool
orbmgr::fini()
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( init_count_ && --init_count_ == 0 )
        return taomgr_->fini() == 0;

    return false;
}

bool
orbmgr::wait()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( thread_ ) {
        thread_->join();
        return true;
    }
    return false;
}

CORBA::ORB_ptr
orbmgr::orb()
{
    return taomgr_->orb();
}

PortableServer::POA_ptr
orbmgr::root_poa()
{
    if ( taomgr_ )
        return taomgr_->root_poa();
    return 0;
}

PortableServer::POA_ptr
orbmgr::child_poa()
{
    if ( taomgr_ )
        return taomgr_->child_poa();
    return 0;
}

PortableServer::POAManager_ptr
orbmgr::poa_manager()
{
    if ( taomgr_ )
        return taomgr_->poa_manager();
    return 0;
}

// static
std::string
orbmgr::activate( PortableServer::Servant servant )
{
    if ( auto mgr = orbmgr::instance() ) {
        CORBA::String_var id = mgr->taomgr_->activate( servant );
        return std::string ( id.in() );
    }
    return "";
}

// static
void
orbmgr::deactivate( const std::string& id )
{
    if ( auto mgr = orbmgr::instance() ) {
        if ( mgr->taomgr_ )
            mgr->taomgr_->deactivate( id.c_str() );
    }
}

// static
void
orbmgr::deactivate( CORBA::Object_ptr obj )
{
    if ( orbmgr::instance()->taomgr_ ) {
        PortableServer::POA_ptr poa = orbmgr::instance()->taomgr_->root_poa();
        if ( poa ) {
            try {
                PortableServer::ObjectId_var objid = poa->reference_to_id( obj );
                poa->deactivate_object( objid );
            } catch ( CORBA::Exception& ex ) {
                ADERROR() << ex._info().c_str();
            }
        }
    }
}

// static
void
orbmgr::deactivate( PortableServer::ServantBase * p_servant )
{
    if ( orbmgr::instance()->taomgr_ ) {
        PortableServer::POA_ptr poa = orbmgr::instance()->taomgr_->root_poa();
        if ( poa ) {
            try {
                PortableServer::ObjectId_var objid = poa->servant_to_id( p_servant );
                poa->deactivate_object( objid );
            } catch ( CORBA::Exception& ex ) {
                ADERROR() << ex._info().c_str();
            }
        }
    }
}

void
orbmgr::shutdown()
{
    CORBA::ORB_ptr orb;

    if ( taomgr_ && ( orb = taomgr_->orb() ) ) {
		if ( ! CORBA::is_nil( orb ) ) {
			try {
				orb->shutdown( true );
			} catch( ... ) {
				ADERROR() << "shutdown got an exception";
			}
		}
	}
}

void
orbmgr::run()
{
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        thread_running_ = true;
        cond_.notify_all();
    } while ( 0 );

    try {
        taomgr_->run();
    } catch ( ... ) {
        ADTRACE() << boost::current_exception_diagnostic_information();
    }
	ADTRACE() << "orbmgr::run -- terminated.";
}

bool
orbmgr::spawn()
{
    if ( thread_ == 0 ) {
        std::unique_lock< std::mutex > lock( mutex_ );
        if ( thread_ == 0 ) {

            thread_ = new std::thread( std::bind( &orbmgr::run, this ) );

            while ( !thread_running_ )
                cond_.wait( lock );
            
            return true;
        }
    }
    return false;
}

