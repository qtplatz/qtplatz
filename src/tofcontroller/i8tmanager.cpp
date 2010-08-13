//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "i8tmanager.h"
#include "i8ttask.h"

using namespace tofcontroller;

i8tManager_i::i8tManager_i(void)
{
	pTask_.reset( new i8tTask() );
}

i8tManager_i::~i8tManager_i(void)
{
}

CORBA::WChar * 
i8tManager_i::software_revision (void)
{
    return L"1.0.0.0";
}

CORBA::Boolean 
i8tManager_i::connect( Receiver_ptr receiver, const CORBA::WChar * toke )
{
    return false;
}

CORBA::Boolean 
i8tManager_i::disconnect ( Receiver_ptr receiver )
{
    return false;
}

CORBA::ULong 
i8tManager_i::get_status (void)
{
    return 0;
}

CORBA::Boolean 
i8tManager_i::initialize (void)
{
    return false;
}

CORBA::Boolean 
i8tManager_i::shutdown (void)
{
    return false;
}

CORBA::Boolean 
i8tManager_i::echo (const char * msg)
{
    return false;
}

CORBA::Boolean 
i8tManager_i::shell (const char * cmdline)
{
    return false;
}

CORBA::Boolean 
i8tManager_i::prepare_for_run ( ControlMethod::Method_ptr m)
{
    return false;
}

CORBA::Boolean 
i8tManager_i::push_back ( SampleBroker::SampleSequence_ptr s)
{
    return false;
}

CORBA::Boolean 
i8tManager_i::event_out ( CORBA::ULong event)
{
    return false;
}

CORBA::Boolean 
i8tManager_i::start_run (void)
{
    return false;
}

CORBA::Boolean 
i8tManager_i::suspend_run (void)
{
    return false;
}

CORBA::Boolean 
i8tManager_i::resume_run (void)
{
    return false;
}

CORBA::Boolean 
i8tManager_i::stop_run (void)
{
    return false;
}

