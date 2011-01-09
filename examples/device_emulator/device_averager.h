// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#pragma once

#include "device_state.h"
#pragma warning(disable:4996)
#include <ace/Event_Handler.h>
#include <ace/Time_Value.h>
#pragma warning(default:4996)
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
        ACE_Time_Value uptime_;
	};

}
