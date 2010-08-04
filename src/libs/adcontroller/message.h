// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

class ACE_OutputCDR;
class ACE_InputCDR;
class ACE_Time_Value;

namespace ns_adcontroller {

	class Message {
	public:
		Message(unsigned long srcId = 0, unsigned long dstId = 0
			   , unsigned long cmdId = 0, unsigned long seqId = 0 );
		~Message(void);
		unsigned long srcId_;
        unsigned long dstId_;
        unsigned long cmdId_;
		unsigned long seqId_;
	};

	ACE_OutputCDR& operator << ( ACE_OutputCDR&, const Message& m );
	ACE_InputCDR& operator >> ( ACE_InputCDR&, Message& m );
	ACE_OutputCDR& operator << ( ACE_OutputCDR&, const ACE_Time_Value& m );
	ACE_InputCDR& operator >> ( ACE_InputCDR&, ACE_Time_Value& );

	enum eNotify {
		Notify_Timeout = 512,
		Notify_Last
	};

	enum eCommand {
		Command_Zero = 1024,
		Command_Last
	};

}
