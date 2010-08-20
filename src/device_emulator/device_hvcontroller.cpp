//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "device_hvcontroller.h"
#include <ace/Message_Block.h>
#include <acewrapper/inputcdr.h>
#include "../tofcontroller/tofcontrollerC.h"
#include "../tofcontroller/constants.h"

using namespace device_emulator;

device_hvcontroller::device_hvcontroller(void)
{
	// adConfig_.reset( new TOFInstrument::ADConfigurations() );
    data_.reset( new AnalyzerDeviceData() );
}

device_hvcontroller::~device_hvcontroller(void)
{
}

device_hvcontroller::device_hvcontroller( const device_hvcontroller& t )
{
	data_ = t.data_;
}

bool
device_hvcontroller::operator == ( const device_hvcontroller& ) const 
{
	return true;
}

bool
device_hvcontroller::instruct_handle_data( ACE_InputCDR& cdr, unsigned long cmdId )
{
	if ( cmdId == tofcontroller::constants::SESSION_SENDTO_DEVICE ) {
       unsigned long clsId;
	   acewrapper::InputCDR in( cdr );
       in >> clsId;
	   if ( clsId == TOFConstants::ClassID_AnalyzerDeviceData ) {
		   return true;
	   }
	}
	return false;
}
