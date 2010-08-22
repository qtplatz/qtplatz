// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "device_state.h"
#include <ace/Event_Handler.h>
class ACE_Message_Block;
class ACE_InputCDR;
class ACE_OutputCDR;

namespace device_emulator {

	class device_averager : public device_state
                          , public ACE_Event_Handler {
	public:
		~device_averager( void );
        device_averager( void );
        device_averager( const device_averager& );
		const char * deviceType() const { return "averager"; }
        void activate();
        void deactivate();

        // ACE_Event_Handler
        virtual int handle_timeout( const ACE_Time_Value&, const void * );
        //<--

		struct handleIt {
			virtual void operator()( bool ) const {}
		};

		bool instruct_handle_data( ACE_InputCDR&, unsigned long cmdId );
        bool instruct_copy_data( ACE_OutputCDR&, ACE_InputCDR&, unsigned long clsid );

		// trigger disarmed after current averaging
		bool instruct_average_stop( handleIt& = handleIt() );  
    
		// trigger armed immediately
		bool instruct_average_start();  
	};

}
