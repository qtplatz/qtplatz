// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "device_state.h"
#include <boost/smart_ptr.hpp>

class ACE_Message_Block;
class ACE_InputCDR;

namespace device_emulator {

	struct AnalyzerDeviceData {
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
	};

	class device_hvcontroller : public device_state {
	public:
		~device_hvcontroller( void );
		device_hvcontroller( void );
		device_hvcontroller( const device_hvcontroller& );
		bool operator == ( const device_hvcontroller& ) const;
		const char * deviceType() const { return "analyzer"; }

		bool instruct_handle_data( ACE_InputCDR&, unsigned long cmdId );

	private:
		bool copyIn( ACE_InputCDR&, AnalyzerDeviceData& );
		// boost::shared_ptr< TOFInstrument::ADConfigurations > adConfig_;
		boost::shared_ptr< AnalyzerDeviceData > data_;
	};

}

