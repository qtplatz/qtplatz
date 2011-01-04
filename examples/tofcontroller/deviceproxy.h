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
// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#pragma warning(disable:4996)
#include <ace/INET_Addr.h>
#include <ace/Event_Handler.h>
#include <ace/Message_Queue.h>
#pragma warning(default:4996)
#include <adportable/protocollifecycle.h>

class ACE_Message_Block;
class ACE_Time_Value;
class TAO_InputCDR;
class TAO_OutputCDR;

namespace acewrapper {
    class DgramHandler;
    class OutputCDR;
}

namespace tofcontroller {

	class TOFTask;

	class DeviceProxy : public ACE_Event_Handler {
    public:
        DeviceProxy( const ACE_INET_Addr&, TOFTask * );

        // void notify_timeout( const ACE_Time_Value& );

		inline const std::wstring& name() { return name_; }
        bool initialize_dgram();
       
        static DeviceProxy * check_hello_and_create( ACE_Message_Block * mb
			                                       , const ACE_INET_Addr& from
												   , TOFTask * task);

        TAO_OutputCDR& prepare_data( TAO_OutputCDR& );
		bool sendto( const ACE_Message_Block * );

		// ACE_Event_Handler
		virtual ACE_HANDLE get_handle() const;
		virtual int handle_input( ACE_HANDLE );
        virtual int handle_timeout( const ACE_Time_Value&, const void * );

    private:
        void handle_lifecycle_mcast( const adportable::protocol::LifeCycleFrame&
			                       , const adportable::protocol::LifeCycleData& );
        void handle_lifecycle_dgram( ACE_Message_Block * );
		bool handle_data( unsigned long classid, TAO_InputCDR& );

    private:
        unsigned short remote_sequence_;
        unsigned short local_sequence_;

		std::wstring name_;
		TOFTask *pTask_;

        boost::shared_ptr< acewrapper::DgramHandler > dgram_handler_;

        ACE_INET_Addr remote_addr_;
        adportable::protocol::LifeCycle lifeCycle_;
        std::string remote_addr_string_;
        std::string local_addr_string_;
    };

}
