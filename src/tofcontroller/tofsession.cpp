//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "tofsession.h"
#include "i8ttask.h"
#include "marshal.hpp"
#include "constants.h"
#include <acewrapper/mutex.hpp>

using namespace tofcontroller;

tofSession_i::tofSession_i(void) : status_current_( Instrument::eNothing )
{
	pTask_.reset( new i8tTask() );
}

tofSession_i::~tofSession_i(void)
{
}



//////////////////////////////////
CORBA::WChar * 
tofSession_i::software_revision (void)
{
	return CORBA::wstring_dup(L"1.0.0.0");
}

CORBA::Boolean 
tofSession_i::setConfiguration( const CORBA::WChar * xml )
{
	if ( pTask_->setConfiguration( xml ) ) {
		pTask_->open();
		status_current_ = ::Instrument::eNotConnected;
		return true;
	}
	return false;
}

CORBA::Boolean 
tofSession_i::configComplete()
{
	status_current_ = ::Instrument::eNotConnected;
    return true;
}

CORBA::Boolean 
tofSession_i::connect( Receiver_ptr receiver, const CORBA::WChar * token )
{
    ACE_UNUSED_ARG( token );
	return pTask_->connect( receiver );
}

CORBA::Boolean 
tofSession_i::disconnect ( Receiver_ptr receiver )
{
	return pTask_->disconnect( receiver );
}

CORBA::ULong 
tofSession_i::get_status (void)
{
    return status_current_;
}

CORBA::Boolean 
tofSession_i::initialize (void)
{
	using namespace constants;
	ACE_Message_Block * mb = marshal<long>::put( SESSION_INITIALIZE, MB_COMMAND );
    pTask_->putq( mb );
	return true;
}

CORBA::Boolean 
tofSession_i::shutdown (void)
{
	pTask_->close();
    return true;
}

CORBA::Boolean 
tofSession_i::echo (const char * msg)
{
    ACE_UNUSED_ARG(msg);
    return false;
}

CORBA::Boolean 
tofSession_i::shell (const char * cmdline)
{
    ACE_UNUSED_ARG( cmdline );
    return false;
}

CORBA::Boolean 
tofSession_i::prepare_for_run ( ControlMethod::Method_ptr m )
{
    ACE_UNUSED_ARG( m );
    return false;
}

CORBA::Boolean 
tofSession_i::push_back ( SampleBroker::SampleSequence_ptr s )
{
    ACE_UNUSED_ARG( s );
    return false;
}

CORBA::Boolean 
tofSession_i::event_out ( CORBA::ULong event)
{
    ACE_UNUSED_ARG( event );
    return false;
}

CORBA::Boolean 
tofSession_i::start_run (void)
{
    return false;
}

CORBA::Boolean 
tofSession_i::suspend_run (void)
{
    return false;
}

CORBA::Boolean 
tofSession_i::resume_run (void)
{
    return false;
}

CORBA::Boolean 
tofSession_i::stop_run (void)
{
    return false;
}

///////////  TOFInstrument::TofSession implementation
CORBA::WChar * 
tofSession_i::tof_software_revision (void)
{
	return CORBA::wstring_dup(L"1.0.0.0");
}

void
tofSession_i::tof_debug( const CORBA::WChar * text, const CORBA::WChar * key )
{
    ACE_OutputCDR cdr;
    cdr.write_wstring( text );
    cdr.write_wstring( key );
    ACE_Message_Block * mb = cdr.begin()->duplicate();
    mb->msg_type( constants::MB_DEBUG );
    pTask_->putq( mb );
}