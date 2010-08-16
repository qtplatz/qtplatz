//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "iproxy.h"
#include "marshal.hpp"
#include "constants.h"
#include "ibroker.h"
#include "manager_i.h"
#include <acewrapper/nameservice.h>


using namespace adcontroller;

iProxy::iProxy( iBroker& t ) : broker_( t )
                             , objref_( false )
{
}

void
iProxy::setConfiguration( const adportable::Configuration& c )
{
    config_ = c;
    if ( config_.attribute( L"type" ) == L"object_ref" ) { // CORBA Object

        name_ = config_.name();
        std::wstring nsname = config_.attribute( L"ns_name" );

        if ( ! nsname.empty() ) {
            CORBA::ORB_var orb = adcontroller::singleton::manager::instance()->getServantManager()->orb();
            CORBA::Object_var obj = acewrapper::NS::resolve_name( orb, nsname );
            if ( ! CORBA::is_nil( obj.in() ) ) {
                impl_ = Instrument::Session::_narrow( obj );
                if ( ! CORBA::is_nil( obj.in() ) ) 
                    objref_ = true;
            }
        }
    }
}

// POA_Receiver
void
iProxy::message( ::Receiver::eINSTEVENT msg, CORBA::ULong value )
{
}

// POA_Receiver
void
iProxy::log( const EventLog::LogMessage& log )
{
	ACE_Message_Block * mb = marshal<::EventLog::LogMessage>::put( log, constants::MB_EVENTLOG );
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
	return true;
}


bool
iProxy::prepare_for_run( const SampleBroker::SampleSequenceLine&, const ControlMethod::Method& )
{
	return true;
}

bool
iProxy::startRun()
{
	return true;
}

bool
iProxy::suspendRun()
{
	return true;
}

bool
iProxy::resumeRun()
{
	return true;
}

bool
iProxy::stopRun()
{
	return true;
}

       
unsigned long
iProxy::getStatus()
{
	return true;
}


