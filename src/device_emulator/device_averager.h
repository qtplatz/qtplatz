// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "device_state.h"
class ACE_Message_Block;
class ACE_InputCDR;

namespace device_emulator {

	class device_averager : public device_state {
	public:
		device_averager(void);
		~device_averager(void);

		bool operator == ( const device_averager& ) const { return true; }
		const char * deviceType() const { return "averager"; }

		struct handleIt {
			virtual void operator()( bool ) const {}
		};

		bool instruct_handle_data( ACE_InputCDR&, unsigned long cmdId );

		// trigger disarmed after current averaging
		bool instruct_average_stop( handleIt& = handleIt() );  
    
		// trigger armed immediately
		bool instruct_average_start();  

	};

}
