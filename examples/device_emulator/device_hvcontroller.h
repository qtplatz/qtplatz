// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "device_state.h"
#include <boost/smart_ptr.hpp>
#pragma warning(disable:4996)
#include <ace/Event_Handler.h>
#pragma warning(default:4996)

class ACE_Message_Block;
class ACE_InputCDR;
class ACE_OutputCDR;

namespace device_emulator {

	struct AnalyzerDeviceData {
        AnalyzerDeviceData() {}
        AnalyzerDeviceData( const AnalyzerDeviceData& );
        void operator = (const AnalyzerDeviceData& );
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

    class device_hvcontroller : public device_state
                              , public ACE_Event_Handler {
	public:
		~device_hvcontroller( void );
		device_hvcontroller( void );
		device_hvcontroller( const device_hvcontroller& );

		const char * deviceType() const { return "analyzer"; }

        // device_state
        virtual void activate();
        virtual void deactivate();

		bool instruct_handle_data( ACE_InputCDR&, unsigned long cmdId );
        bool instruct_copy_data( ACE_OutputCDR&, ACE_InputCDR&, unsigned long clsid );

        // ACE_Event_Handler
        virtual int handle_timeout( const ACE_Time_Value&, const void * );

	private:
		bool copyIn( ACE_InputCDR&, AnalyzerDeviceData& );
        bool copyOut( ACE_OutputCDR&, const AnalyzerDeviceData& );
		// boost::shared_ptr< TOFInstrument::ADConfigurations > adConfig_;
		boost::shared_ptr< AnalyzerDeviceData > data_;
	};

}

