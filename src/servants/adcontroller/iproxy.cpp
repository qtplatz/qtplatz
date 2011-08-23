// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
#include "marshal.hpp"
#include "constants.hpp"
#include "ibroker.hpp"
#include "manager_i.hpp"
#include <acewrapper/brokerhelper.hpp>
#include <adportable/string.hpp>
#include <adportable/debug.hpp>
#include <stdexcept>

using namespace adcontroller;

iProxy::iProxy( iBroker& t ) : objref_( false )
			     , objId_(0) 
                             , broker_( t )
{
}

void
iProxy::setConfiguration( const adportable::Configuration& c )
{
    config_ = c;
    if ( config_.attribute( L"type" ) == L"object_ref" ) { // CORBA Object

        name_ = config_.name();
        std::string nsname = adportable::string::convert( config_.attribute( L"ns_name" ) );
        CORBA::ORB_var orb = adcontroller::singleton::manager::instance()->orb();
        std::string iorBroker = adcontroller::singleton::manager::instance()->broker_manager_ior();

        if ( ! nsname.empty() ) {
            Broker::Manager_var mgr = acewrapper::brokerhelper::getManager( orb, iorBroker );
            if ( CORBA::is_nil( mgr ) )
                throw std::runtime_error( "iProxy::setConfiguration -- can't get Broker::Manager reference" );

            std::string ior = mgr->ior( nsname.c_str() );
            if ( ! ior.empty() ) {
                try {
                    // acewrapper::NS::resolve_name( orb, nsname );
                    CORBA::Object_var obj = orb->string_to_object( ior.c_str() );
                    if ( ! CORBA::is_nil( obj.in() ) ) {
                        impl_ = Instrument::Session::_narrow( obj );
                        if ( ! CORBA::is_nil( impl_ ) ) 
                            objref_ = true;
                    } 
                } catch ( CORBA::Exception& ex ) {
                    adportable::debug() << "adcontroller::iproxy::setConfiguration '" << nsname << "' " << ex._info().c_str();
                }
            } else {
                adportable::debug() << "iProxy::setConfiguration -- object '" << nsname << "' not registerd";
            }
        }
    }
}

// POA_Receiver
void
iProxy::message( ::Receiver::eINSTEVENT msg, CORBA::ULong value )
{
    TAO_OutputCDR cdr;
    cdr << name_.c_str();
    cdr << msg;
    cdr << value;
    ACE_Message_Block * mb = cdr.begin()->duplicate();
    mb->msg_type( constants::MB_MESSAGE );
    broker_.putq( mb );
}

// POA_Receiver
void
iProxy::log( const EventLog::LogMessage& log )
{
    ACE_Message_Block * mb = marshal<EventLog::LogMessage>::put( log, constants::MB_EVENTLOG );
    broker_.putq( mb );
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
iProxy::connect( const std::wstring& token )
{
    if ( objref_ )
        return impl_->connect( this->_this(), token.c_str() );
    return false;
}

bool
iProxy::initialize()
{
    // in order to catch up all event, connect first, and then initialize
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
iProxy::prepare_for_run( const SampleBroker::SampleSequenceLine&, const ControlMethod::Method& m )
{
    return impl_->prepare_for_run( const_cast<ControlMethod::Method *>(&m) );
}

bool
iProxy::startRun()
{
    return impl_->start_run();
}

bool
iProxy::suspendRun()
{
    return impl_->suspend_run();
}

bool
iProxy::resumeRun()
{
    return impl_->resume_run();
}

bool
iProxy::stopRun()
{
    return impl_->stop_run();
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
