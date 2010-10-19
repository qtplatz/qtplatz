//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "iproxy.h"
#include "marshal.hpp"
#include "constants.h"
#include "ibroker.h"
#include "manager_i.h"
#include <adportable/string.h>

using namespace adcontroller;

iProxy::iProxy( iBroker& t ) : broker_( t )
                             , objref_( false )
							 , objId_(0) 
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
            CORBA::Object_var objMgr = orb->string_to_object( adcontroller::singleton::manager::instance()->broker_manager_reference() );
            Broker::Manager_var mgr = Broker::Manager::_narrow( objMgr );
            if ( ! CORBA::is_nil( mgr.in() ) ) {
                CORBA::Object_var obj = orb->string_to_object( mgr->ior( adportable::string::convert( name_ ).c_str() ) ); // acewrapper::NS::resolve_name( orb, nsname );
                if ( ! CORBA::is_nil( obj.in() ) ) {
                    try {
                        impl_ = Instrument::Session::_narrow( obj );
                        if ( ! CORBA::is_nil( impl_ ) ) 
                            objref_ = true;
                    } catch ( CORBA::Exception& ) {
                    }
                }
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
