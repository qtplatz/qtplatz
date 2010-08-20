//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "device_averager.h"
#include <ace/Message_Block.h>
#include "../tofcontroller/tofcontrollerC.h"
#include "../tofcontroller/constants.h"

using namespace device_emulator;

device_averager::device_averager(void)
{
}

device_averager::~device_averager(void)
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
