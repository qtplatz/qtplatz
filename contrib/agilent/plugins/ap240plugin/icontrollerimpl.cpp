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
#include <adportable/scoped_debug.hpp>
#include <adportable/serializer.hpp>
#include <adicontroller/manager.hpp>
#include <QLibrary>
#include <memory>

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

    class iControllerImpl::impl {
    public:
        std::shared_ptr< adicontroller::Instrument::Session > session_;
        std::shared_ptr< adicontroller::Receiver > receiver_;
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
    setInitialized( true );
    // if ( ( impl_->session_ = session->pThis() ) ) {
    //     QString xml;
    //     if ( document::instance()->getMethodXml( xml ) )
    //         impl_->session_->setConfiguration( xml.toUtf8().toStdString() );
        
    //     if ( ( impl_->receiver_ = std::make_shared< ReceiverImpl >() ) )
    //         impl_->session_->connect( impl_->receiver_.get(), "token" );
    // }
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
iControllerImpl::preparing_for_run( adcontrols::ControlMethod& cm )
{
    return false;
}
