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
#include <adportable/string.hpp>
#include <adlog/logger.hpp>
#include <stdexcept>
#include <functional>

using namespace adcontroller;

iProxy::iProxy( iTask& ) : objref_( false )
                         , isConnected_( false )
						 , objId_(0)
{
}

// setConfiguration call from iBroker when configComplete is called.
// which is called when qtplatz fire 'extentionInstalled()'
bool
iProxy::initialConfiguration( const adportable::Configuration& c )
{
    config_ = c;
    std::string nsname = config_.attribute( "ns_name" );
    name_ = adportable::string::convert( config_.name() );
    
    if ( config_.attribute( "type" ) == "object_reference" ) { // CORBA Object

        if ( ! nsname.empty() ) {

			Broker::Manager_var bmgr = adcontroller::manager_i::instance()->impl().getBrokerManager();
            if ( CORBA::is_nil( bmgr ) )
                throw std::runtime_error( "iProxy::init_object_ref -- can't get Broker::Manager reference" );
            try {
                CORBA::Object_var obj = bmgr->find_object( nsname.c_str() );
                if ( ! CORBA::is_nil( obj.in() ) ) {
                    impl_ = Instrument::Session::_narrow( obj );
                    if ( ! CORBA::is_nil( impl_ ) ) {
                        if ( impl_->echo( "hello" ) )
                            objref_ = true;
                    }
                } 
            } catch ( CORBA::Exception& ex ) {
                ADERROR() << "adcontroller::iproxy::setConfiguration '" << nsname << "' " << ex._info().c_str();
            }
        }
    } else {
        ADERROR() << "iProxy::setConfiguration -- object '" << nsname << "' not registerd";
    }
    return objref_;
}

// POA_Receiver
void
iProxy::message( ::Receiver::eINSTEVENT msg, CORBA::ULong value )
{
    unsigned long msgId = static_cast< unsigned long >( msg );
	// messaged need to be cashed here for exact state change of the control server
	iTask::instance()->io_service().post( std::bind(&iTask::handle_message, iTask::instance(), name_, msgId, value ) );
}

// POA_Receiver
void
iProxy::log( const EventLog::LogMessage& log )
{
    iTask::instance()->io_service().post( std::bind(&iTask::handle_eventlog, iTask::instance(), log ) );
}

// POA_Receiver
void
iProxy::shutdown()
{
    // connection shoutdown ack.
    // do nothing
}

// POA_Receiver
void
iProxy::debug_print( CORBA::Long pri, CORBA::Long cat, const char * text )
{
    ACE_UNUSED_ARG(pri);
    ACE_UNUSED_ARG(cat);
    ACE_UNUSED_ARG(text);
}


// iProxy
void
iProxy::reset_clock()
{
}

bool
iProxy::connect( const std::string& token )
{
	if (objref_) {
		isConnected_ = impl_->connect(this->_this(), token.c_str());
		return isConnected_;
	}
    return false;
}

bool
iProxy::disconnect()
{
    if ( objref_ ) {
		try {
			impl_->disconnect(this->_this());
			isConnected_ = false;
			return true;
		} catch ( CORBA::Exception& ex ) {
			ADERROR() << "iProxy::disconnect got an exception: " << ex._info().c_str();
		}
	}
    return false;
}

bool
iProxy::initialize()
{
    if ( objref_ )
        return impl_->initialize();
    return false;
}

bool
iProxy::request_shutdown()
{
    if ( objref_ )
        return impl_->shutdown();
    return false;
}

bool
iProxy::eventOut( unsigned long event )
{
    if ( objref_ )
        return impl_->event_out( event );
    return false;
}


bool
iProxy::prepare_for_run( const ControlMethod::Method& m )
{
    if ( objref_ )
        return impl_->prepare_for_run( m );
    return false;
}

bool
iProxy::startRun()
{
    if ( objref_ )
        return impl_->start_run();
    return false;
}

bool
iProxy::suspendRun()
{
    if ( objref_ )
        return impl_->suspend_run();
    return false;
}

bool
iProxy::resumeRun()
{
    if ( objref_ )
        return impl_->resume_run();
    return false;
}

bool
iProxy::stopRun()
{
    if ( objref_ )
        return impl_->stop_run();
    return false;
}

       
unsigned long
iProxy::getStatus()
{
    return impl_->get_status();
}

Instrument::Session_ptr
iProxy::getSession()
{
    return Instrument::Session::_duplicate( impl_ );
}

void
iProxy::objId( unsigned long id )
{
    objId_ = id;
}

unsigned long
iProxy::objId() const
{
    return objId_;
}
