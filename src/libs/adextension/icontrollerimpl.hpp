/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include <adextension/icontroller.hpp>
#include <adicontroller/instrument.hpp>
#include <adicontroller/receiver.hpp>
#include <adicontroller/signalobserver.hpp>
#include <condition_variable>
#include <vector>
#include <memory>

namespace adextension {

    class iControllerImpl : public adextension::iController {
    public:

        class ReceiverImpl : public adicontroller::Receiver {
            std::weak_ptr< iController > controller_;
        public:
            ~ReceiverImpl() {}
            ReceiverImpl( iController * p ) : controller_( p->pThis() ) {}
            
            void message( eINSTEVENT msg, uint32_t value ) override {
                if ( auto p = controller_.lock() )
                    emit p->message( p.get(), unsigned int( msg ), unsigned int( value ) );
            }
            
            void log( const adicontroller::EventLog::LogMessage& log ) override {
            }
            
            void shutdown() override {
            }
            
            void debug_print( uint32_t priority, uint32_t category, const std::string& text ) override {
            }
        };

        /*
         *  ObserverEvents -- implementation helper
        */
        class ObserverEventsImpl : public adicontroller::SignalObserver::ObserverEvents {
            // ObserverEvents
            std::weak_ptr< iController > controller_;
        public:
            ObserverEventsImpl( iController * p ) : controller_( p->pThis() ) {}
            
            void OnConfigChanged( uint32_t objId, adicontroller::SignalObserver::eConfigStatus status ) { }
            
            void OnUpdateData( uint32_t objId, long pos ) { }
            
            void OnMethodChanged( uint32_t objId, long pos ) { }
            
            void OnEvent( uint32_t objId, uint32_t event, long pos ) { }
            
            void onDataChanged( adicontroller::SignalObserver::Observer * so, uint32_t pos ) override {
                if ( auto p = controller_.lock() ) {
                    auto self( so->shared_from_this() );
                    emit p->dataChanged( so, pos );
                }
            }
        };

    private:
        iControllerImpl( const iControllerImpl& ) = delete;
        iControllerImpl& operator = (const iControllerImpl& ) = delete;
    public:
        // iControllerImpl
        iControllerImpl( const QString& module_name = QString()
                         , int module_number = 1 ) : isInitialized_( false )
                                                   , module_name_( module_name )
                                                   , module_number_ ( module_number ) {
        }

        template< typename T > std::shared_ptr<T> create( const QString& modle_name, int number ) {
            return std::make_shared<T>( module_name, number );
        }

        ~iControllerImpl() {
            if ( session_ ) {
                if ( observerEvents_ ) {
                    if ( auto observer = session_->getObserver()->shared_from_this() )
                        observer->disconnect( observerEvents_.get() );
                }
                if ( receiver_ )
                    session_->disconnect( receiver_.get() );
            }
        }

        // connect() should be implemented specific to own instance

        void connect( adicontroller::Instrument::Session * session, const char * token ) {
            if ( session_ = session->pThis() ) {
                setInitialized( true );
                if ( receiver_ = std::make_shared< ReceiverImpl >( this ) ) {
                    session_->connect( receiver_.get(), token );
                    if ( auto observer = session_->getObserver() ) {
                        if ( observerEvents_ = std::make_shared< ObserverEventsImpl >( this ) )
                            observer->connect( observerEvents_.get(), adicontroller::SignalObserver::Realtime, token );
                    }
                    emit connected( this );
                }
            }
        }
        
        virtual void setInitialized( bool v ) {
            std::lock_guard< std::mutex > lock( mutex_ );
            isInitialized_ = v;
            cv_.notify_all();
        }

        bool wait_for_connection_ready() override {
            std::unique_lock< std::mutex > lock( mutex_ );
            while ( !isInitialized_ )
                cv_.wait( lock );
            return isInitialized_;
        }

        adicontroller::Instrument::Session * getInstrumentSession() override {
            return session_.get();
        }

        QString module_name() const override { return module_name_; }
        int module_number() const override { return module_number_; }

    protected:
        bool isInitialized_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::shared_ptr< adicontroller::Instrument::Session > session_;
        std::shared_ptr< ReceiverImpl > receiver_;
        std::shared_ptr< ObserverEventsImpl > observerEvents_;
        QString module_name_;
        int module_number_;
    };

}

