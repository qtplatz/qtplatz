// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <ace/INET_Addr.h>
#include <ace/Event_Handler.h>
#include <ace/Message_Queue.h>
#include <adportable/protocollifecycle.h>

class ACE_Message_Block;
class ACE_Time_Value;
class TAO_InputCDR;

namespace acewrapper {
    class DgramHandler;
}

namespace tofcontroller {

	class i8tTask;

	class DeviceProxy : public ACE_Event_Handler {
    public:
        DeviceProxy( const ACE_INET_Addr&, i8tTask * );

        // void notify_timeout( const ACE_Time_Value& );

		inline const std::wstring& name() { return name_; }
        bool initialize_dgram();
       
        static DeviceProxy * check_hello_and_create( ACE_Message_Block * mb
			                                       , const ACE_INET_Addr& from
												   , i8tTask * task);

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
		i8tTask *pTask_;

        boost::shared_ptr< acewrapper::DgramHandler > dgram_handler_;

        ACE_INET_Addr remote_addr_;
        adportable::protocol::LifeCycle lifeCycle_;
        std::string remote_addr_string_;
        std::string local_addr_string_;
    };

}
