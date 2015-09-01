/**************************************************************************
** Copyright (C) 2014-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2014-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "icontrollerimpl.hpp"
#include "document.hpp"
#include <adplugin/plugin.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adicontroller/instrument.hpp>
#include <adicontroller/receiver.hpp>
#include <adicontroller/signalobserver.hpp>
#include <adportable/scoped_debug.hpp>
#include <adportable/serializer.hpp>
#include <adicontroller/manager.hpp>
#include <QLibrary>
#include <memory>

#if defined _DEBUG || defined DEBUG
# if defined WIN32
#  define DEBUG_LIB_TRAIL "d" // xyzd.dll
# elif defined __MACH__
#  define DEBUG_LIB_TRAIL "_debug" // xyz_debug.dylib
# else
#  define DEBUG_LIB_TRAIL ""        // xyz.so 
# endif
#else
# define DEBUG_LIB_TRAIL ""
#endif

namespace ap240 {

    class ReceiverImpl : public adicontroller::Receiver {
    public:
            
        ~ReceiverImpl() {
        }
            
        void message( eINSTEVENT msg, uint32_t value ) override {
        }
            
        void log( const adicontroller::EventLog::LogMessage& log ) override {
        }
            
        void shutdown() override {
        }
            
        void debug_print( uint32_t priority, uint32_t category, const std::string& text ) override {
        }
    };

    class ObserverEventsImpl : public adicontroller::SignalObserver::ObserverEvents {
    public:
            
    protected:
        // ObserverEvents
        void OnConfigChanged( uint32_t objId, adicontroller::SignalObserver::eConfigStatus status ) {} // depricated

        void OnUpdateData( uint32_t objId, long pos ) {}                   // depricated

        void OnMethodChanged( uint32_t objId, long pos ) {}                // depricated

        void OnEvent( uint32_t objId, uint32_t event, long pos ) {}        // depricated

        void onDataChanged( adicontroller::SignalObserver::Observer * so, uint32_t pos ) {
            // task::instance()->onDataChanged( so, pos );
        }
    };

    class iControllerImpl::impl {
    public:
        std::shared_ptr< adicontroller::Instrument::Session > session_;
        std::shared_ptr< adicontroller::Receiver > receiver_;
        std::shared_ptr< adicontroller::SignalObserver::Observer > observer_;
        std::shared_ptr< ObserverEventsImpl > observerEvents_;
    };

}

using namespace ap240;

iControllerImpl::iControllerImpl() : isInitialized_( false )
                                   , impl_( new impl() )
{
}

iControllerImpl::~iControllerImpl()
{
    delete impl_;
}

void
iControllerImpl::setInitialized( bool v )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    isInitialized_ = v;
    cv_.notify_all();
}

bool
iControllerImpl::connect()
{
    typedef adplugin::plugin * ( *factory )( );

    QLibrary lib( QString("ap240controller") + DEBUG_LIB_TRAIL );

    if ( ! lib.isLoaded() )
        lib.load();
    
    if ( lib.isLoaded() ) {
        
        if ( factory f = reinterpret_cast<factory>( lib.resolve( "adplugin_plugin_instance" ) ) ) {
            
            adplugin::plugin * plugin = f();
            if ( auto manager = plugin->query_interface< adicontroller::manager >() ) {
                if ( auto session = manager->session( "ap240::icontrollerimpl" ) ) {
                    
                    if ( ( impl_->session_ = session->pThis() ) ) {

                        setInitialized( true );

                        if ( ( impl_->receiver_ = std::make_shared< ReceiverImpl >() ) ) {
                            
                            // document::instance()->task_initialize();
                            
                            if ( impl_->session_->connect( impl_->receiver_.get(), "ap240::iControllerImpl" ) ) {
                                emit connected( this );
                                if ( auto observer = impl_->session_->getObserver() ) {
                                    if ( impl_->observer_ = observer->shared_from_this() ) {
                                        impl_->observerEvents_ = std::make_shared< ObserverEventsImpl >();
                                        impl_->observer_->connect( impl_->observerEvents_.get(), adicontroller::SignalObserver::Realtime, "malpixacquire" );
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return true;

}

bool
iControllerImpl::wait_for_connection_ready()
{
    std::unique_lock< std::mutex > lock( mutex_ );
    while ( !isInitialized_ )
        cv_.wait( lock );
    return isInitialized_;
}

bool
iControllerImpl::preparing_for_run( adcontrols::ControlMethod::Method& cm )
{
    return false;
}

adicontroller::Instrument::Session *
iControllerImpl::getInstrumentSession()
{
    return 0;
}
