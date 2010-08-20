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
           AnalyzerDeviceData data;
		   if ( copyIn( cdr, data ) )
			   return true;
	   }
	}
	return false;
}

///////////
bool
device_hvcontroller::copyIn( ACE_InputCDR& cdr, AnalyzerDeviceData& data )
{
	acewrapper::InputCDR in(cdr);
    in >> data.model;
    in >> data.hardware_rev;
    in >> data.firmware_rev;
    in >> data.serailnumber;
	in >> data.positive_polarity;
    in >> data.ionguide_bias_voltage;
/*
		std::string model;
		std::string hardware_rev;
		std::string firmware_rev;
		std::string serailnumber;
		bool positive_polarity;  // bool
		unsigned long ionguide_bias_voltage;
		unsigned long ionguide_rf_voltage;
		unsigned long orifice1_voltage;
		unsigned long orifice2_voltage;
		unsigned long orifice4_voltage;
		unsigned long focus_lens_voltage;
		unsigned long left_right_voltage;
		unsigned long quad_lens_voltage;
		unsigned long pusher_voltage;
		unsigned long pulling_voltage;
		unsigned long supress_voltage;
		unsigned long pushbias_voltage;
		unsigned long mcp_voltage;
		unsigned long accel_voltage;  // digital value
*/
	return true;
}
