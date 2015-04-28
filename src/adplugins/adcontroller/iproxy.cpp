// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "iproxy.hpp"
#include "task.hpp"
#include "manager_i.hpp"
#include <acewrapper/orbservant.hpp>
#include <adinterface/receiverS.h>
#include <adinterface/instrumentC.h>
#include <adlog/logger.hpp>
#include <adportable/configuration.hpp>
#include <adportable/string.hpp>
#include <stdexcept>
#include <functional>

namespace adcontroller {

    class iProxy::impl : public POA_Receiver {
        impl( const impl& ) = delete;
        void operator = ( const impl& ) = delete;

    public:
        impl() : objId_(0)
            , remote_status_( Instrument::eNothing ) {
        }
        
        uint32_t objId_;
        Instrument::Session_var session_;
        adportable::Configuration config_;
        std::wstring name_;
        Instrument::eInstStatus remote_status_;

        // POA_Receiver
        void message( ::Receiver::eINSTEVENT msg, uint32_t value ) {
            auto msgId = static_cast< unsigned long >( msg );
            if ( msgId == Receiver::STATE_CHANGED ) {
                remote_status_ = static_cast<Instrument::eInstStatus>( value );
            }
            iTask::instance()->io_service().post( std::bind(&iTask::handle_message, iTask::instance(), name_, msgId, value ) );
        }
        
        void log( const EventLog::LogMessage& log ) {
            iTask::instance()->io_service().post( std::bind(&iTask::handle_eventlog, iTask::instance(), log ) );
        }
        
        void shutdown() {
        }

        void debug_print( int32_t pri, int32_t cat, const char * text ) {
            (void)pri; (void)cat; (void)text;
        }
    };
    
}

using namespace adcontroller;

iProxy::~iProxy()
{
    delete impl_;
}

iProxy::iProxy( iTask& t ) : impl_( new impl() )
{
}

// setConfiguration call from iBroker when configComplete is called.
// which is called when qtplatz fire 'extentionInstalled()'
bool
iProxy::initialConfiguration( const adportable::Configuration& c )
{
    impl_->config_ = c;
    std::string nsname = impl_->config_.attribute( "ns_name" );
    impl_->name_ = adportable::string::convert( impl_->config_.name() );
    
    if ( impl_->config_.attribute( "type" ) == "object_reference" ) { // CORBA Object

        if ( ! nsname.empty() ) {

			Broker::Manager_var bmgr = adcontroller::manager_i::instance()->impl().getBrokerManager();
            if ( CORBA::is_nil( bmgr ) )
                throw std::runtime_error( "iProxy::init_object_ref -- can't get Broker::Manager reference" );
            try {
                CORBA::Object_var obj = bmgr->find_object( nsname.c_str() );
                if ( ! CORBA::is_nil( obj.in() ) ) {
                    impl_->session_ = Instrument::Session::_narrow( obj );
                    if ( !CORBA::is_nil( impl_->session_ ) ) {
                        return true;
                    }
                } 
            } catch ( CORBA::Exception& ex ) {
                ADERROR() << "adcontroller::iproxy::setConfiguration '" << nsname << "' " << ex._info().c_str();
            }
        }
    } else {
        ADERROR() << "iProxy::setConfiguration -- object '" << nsname << "' not registerd";
    }
    return false;
}

// iProxy
iProxy::operator bool() const
{
    return !CORBA::is_nil( impl_->session_ );
}


void
iProxy::reset_clock()
{
}

bool
iProxy::connect( const std::string& token )
{
    if ( !CORBA::is_nil( impl_->session_ ) )
        return impl_->session_->connect( impl_->_this(), token.c_str() );
    return false;
}

bool
iProxy::disconnect()
{
    if ( !CORBA::is_nil( impl_->session_ ) ) {
        try {
            return impl_->session_->disconnect( impl_->_this() );
		} catch ( CORBA::Exception& ex ) {
			ADERROR() << "iProxy::disconnect got an exception: " << ex._info().c_str();
		}
	}
    return false;
}

bool
iProxy::initialize()
{
    if ( !CORBA::is_nil( impl_->session_ ) )    
        return impl_->session_->initialize();
    return false;
}

bool
iProxy::request_shutdown()
{
    if ( !CORBA::is_nil( impl_->session_ ) )    
        return impl_->session_->shutdown();
    return false;
}

bool
iProxy::eventOut( unsigned long event )
{
    if ( !CORBA::is_nil( impl_->session_ ) )        
        return impl_->session_->event_out( event );
    return false;
}


bool
iProxy::prepare_for_run( const ControlMethod::Method& m )
{
    if ( !CORBA::is_nil( impl_->session_ ) )
        return impl_->session_->prepare_for_run( m );
    return false;
}

bool
iProxy::startRun()
{
    if ( !CORBA::is_nil( impl_->session_ ) )
        return impl_->session_->start_run();
    return false;
}

bool
iProxy::suspendRun()
{
    if ( !CORBA::is_nil( impl_->session_ ) )
        return impl_->session_->suspend_run();
    return false;
}

bool
iProxy::resumeRun()
{
    if ( !CORBA::is_nil( impl_->session_ ) )
        return impl_->session_->resume_run();
    return false;
}

bool
iProxy::stopRun()
{
    if ( !CORBA::is_nil( impl_->session_ ) )
        return impl_->session_->stop_run();
    return false;
}

       
uint32_t
iProxy::getStatus()
{
    if ( !CORBA::is_nil( impl_->session_ ) )    
        return impl_->session_->get_status();
    return 0;
}

Instrument::Session_ptr
iProxy::getSession()
{
    if ( !CORBA::is_nil( impl_->session_ ) )        
        return Instrument::Session::_duplicate( impl_->session_ );
    return 0;
}

void
iProxy::objId( unsigned long id )
{
    impl_->objId_ = id;
}

uint32_t
iProxy::objId() const
{
    return impl_->objId_;
}
