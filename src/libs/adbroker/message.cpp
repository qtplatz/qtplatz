//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "message.h"
#include <acewrapper/outputcdr.h>
#include <acewrapper/inputcdr.h>

using namespace adbroker;

namespace adbroker {

	ACE_OutputCDR& operator << ( ACE_OutputCDR& cdr, const Message& m )
	{
		acewrapper::OutputCDR ocdr( cdr );
		ocdr << m.srcId_;
		ocdr << m.dstId_;
		ocdr << m.cmdId_;
		ocdr << m.seqId_;
		return cdr;
	}

	ACE_InputCDR& operator >> ( ACE_InputCDR& cdr, Message& m )
	{
		acewrapper::InputCDR icdr( cdr );
		icdr >> m.srcId_;
		icdr >> m.dstId_;
		icdr >> m.cmdId_;
		icdr >> m.seqId_;
		return cdr;
	}

	ACE_OutputCDR& operator << ( ACE_OutputCDR& cdr, const ACE_Time_Value& tv )
	{
		acewrapper::OutputCDR ocdr( cdr );
		ocdr << tv.sec();
		ocdr << tv.sec();
		return cdr;
	}

	ACE_InputCDR& operator >> ( ACE_InputCDR& cdr, ACE_Time_Value& tv )
	{
		acewrapper::InputCDR icdr( cdr );
        time_t sec;
        unsigned long usec;
		icdr >> sec;
		icdr >> usec;
        tv.set( sec, usec );
		return cdr;
	}

}

Message::Message( unsigned long srcId
                , unsigned long dstId
				, unsigned long cmdId
				, unsigned long seqId ) : srcId_(srcId)
				                        , dstId_(dstId)
										, cmdId_(cmdId)
										, seqId_(seqId)
{
}

Message::~Message(void)
{
}

