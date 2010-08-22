//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "device_averager.h"
#include <ace/Message_Block.h>
#include <acewrapper/inputcdr.h>
#include <acewrapper/outputcdr.h>
#include "../tofcontroller/tofcontrollerC.h"
#include "../tofcontroller/constants.h"
#include "devicefacade.h"
#include "constants.h"
#include "./reactor_thread.h"
#include <ace/Reactor.h>


using namespace device_emulator;

device_averager::~device_averager(void)
{
    if ( state_ > device_state::state_initializing )
        deactivate();
}

device_averager::device_averager(void)
{
}

device_averager::device_averager( const device_averager& t ) : device_state( t )
{
}

bool
device_averager::instruct_handle_data( ACE_InputCDR& cdr, unsigned long cmdId )
{
	if ( cmdId == tofcontroller::constants::SESSION_SENDTO_DEVICE ) {
		unsigned long clsId;
		//cdr.read_ulong( clsId );
	}
	return false;
}

bool
device_averager::instruct_copy_data( ACE_OutputCDR&, ACE_InputCDR&, unsigned long )
{
    return false;
}

void
device_averager::activate()
{
    doit( device_state::command_initialize );
    ACE_Reactor * reactor = acewrapper::singleton::ReactorThread::instance()->get_reactor();
    reactor->schedule_timer( this, 0, ACE_Time_Value(1), ACE_Time_Value(1) );
}

void
device_averager::deactivate()
{
    doit( device_state::command_off );
    ACE_Reactor * reactor = acewrapper::singleton::ReactorThread::instance()->get_reactor();
    reactor->cancel_timer( this );
}

int
device_averager::handle_timeout( const ACE_Time_Value&, const void * )
{
    if ( state() <= device_state::state_initializing )
        doit( device_state::command_stop );

    return 0;
}