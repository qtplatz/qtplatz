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
#include <adportable/debug.hpp>
#include <tao/Utils/ORB_Manager.h>
#include <adlog/logger.hpp>
#include <boost/exception/all.hpp>
#include <thread>

namespace adorbmgr {

    class orbmgr::impl {

        impl( const impl& ) = delete;
    public:
        
        ~impl() {
            delete taomgr_;
        }
        
        impl( CORBA::ORB_ptr orb
              , PortableServer::POA_ptr poa
              , PortableServer::POAManager_ptr poamanager ) : thread_running_( false )
                                                            , init_count_( 0 )  
                                                            , thread_( 0 )
                                                            , taomgr_( new TAO_ORB_Manager( orb, poa, poamanager ) ) {
        }
        
		void run() {
            do {
                std::lock_guard< std::mutex > lock( mutex_ );
                thread_running_ = true;
                cond_.notify_all();       // wake-up spawn
            } while(0);
            try {
                taomgr_->run();
            } catch ( ... ) {
                ADTRACE() << boost::current_exception_diagnostic_information();
            }
            ADTRACE() << "orbmgr::run -- terminated.";
        }

        bool spawn() {
            std::unique_lock< std::mutex > lock( mutex_ );
            if ( thread_ == 0 ) {
                thread_ = new adportable::asio::thread( std::bind( &orbmgr::impl::run, this ) );
                while ( !thread_running_ ) // block until run() wake-up
                    cond_.wait( lock );
                return true;
            }
            return false; // already spawned
        }

        bool wait() {
            std::lock_guard< std::mutex > lock( mutex_ );
            if ( thread_ ) {
                thread_->join();
                return true;
            }
            return false;
        }
        
        void shutdown()  {
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
        
        static std::atomic< orbmgr * > instance_;
        static std::mutex mutex_;
        
        std::atomic<bool> thread_running_;
        std::atomic<size_t> init_count_;
        adportable::asio::thread * thread_;
        TAO_ORB_Manager * taomgr_;
        std::condition_variable cond_;        
    };
    
    std::atomic< orbmgr * > orbmgr::impl::instance_(0);
    std::mutex orbmgr::impl::mutex_;
}

using namespace adorbmgr;

orbmgr *
orbmgr::instance()
{
    typedef orbmgr T;
    T * tmp = impl::instance_.load( std::memory_order_relaxed );
    std::atomic_thread_fence( std::memory_order_acquire );
    if ( tmp == nullptr ) {
        std::lock_guard< std::mutex > lock( impl::mutex_ );
        tmp = impl::instance_.load( std::memory_order_relaxed );
        if ( tmp == nullptr ) {
            tmp = new T();
            std::atomic_thread_fence( std::memory_order_release );
            impl::instance_.store( tmp, std::memory_order_relaxed );
        }
    }
    return tmp;
}

orbmgr::~orbmgr()
{
    delete impl_;
    ADTRACE() << "orbmgr deleted cleanly";
}

orbmgr::orbmgr( CORBA::ORB_ptr orb
                , PortableServer::POA_ptr poa
                , PortableServer::POAManager_ptr poamanager ) : impl_( new impl( orb, poa, poamanager ) )
{
}

int
orbmgr::init( int ac, ACE_TCHAR * av[] )
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );

    if ( impl_->init_count_++ == 0 )
        return impl_->taomgr_->init( ac, av );

    return 0;
}

bool
orbmgr::fini()
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );

    if ( impl_->init_count_ && --impl_->init_count_ == 0 )
        return impl_->taomgr_->fini() == 0;

    return false;
}

bool
orbmgr::wait()
{
    return impl_->wait();
}

CORBA::ORB_ptr
orbmgr::orb()
{
    return impl_->taomgr_->orb();
}

PortableServer::POA_ptr
orbmgr::root_poa()
{
    if ( impl_->taomgr_ )
        return impl_->taomgr_->root_poa();
    return 0;
}

PortableServer::POA_ptr
orbmgr::child_poa()
{
    if ( impl_->taomgr_ )
        return impl_->taomgr_->child_poa();
    return 0;
}

PortableServer::POAManager_ptr
orbmgr::poa_manager()
{
    if ( impl_->taomgr_ )
        return impl_->taomgr_->poa_manager();
    return 0;
}

// static
std::string
orbmgr::activate( PortableServer::Servant servant )
{
    if ( auto taomgr = orbmgr::instance()->impl_->taomgr_ ) {
        CORBA::String_var id = taomgr->activate( servant );
        return std::string ( id.in() );
    }
    return "";
}

// static
void
orbmgr::deactivate( const std::string& id )
{
    if ( auto taomgr = orbmgr::instance()->impl_->taomgr_ )
        taomgr->deactivate( id.c_str() );
}

// static
void
orbmgr::deactivate( CORBA::Object_ptr obj )
{
    if ( auto poa = orbmgr::instance()->root_poa() ) {
        try {
            PortableServer::ObjectId_var objid = poa->reference_to_id( obj );
            poa->deactivate_object( objid );
        } catch ( CORBA::Exception& ex ) {
            ADERROR() << ex._info().c_str();
        }
    }
}

// static
void
orbmgr::deactivate( PortableServer::ServantBase * p_servant )
{
    if ( auto poa = orbmgr::instance()->root_poa() ) {    
        try {
            PortableServer::ObjectId_var objid = poa->servant_to_id( p_servant );
            poa->deactivate_object( objid );
        } catch ( CORBA::Exception& ex ) {
            ADERROR() << ex._info().c_str();
        }
    }
}

void
orbmgr::shutdown()
{
    impl_->shutdown();
}

bool
orbmgr::spawn()
{
    return impl_->spawn();
}

