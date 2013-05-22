// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "message.hpp"
#include <acewrapper/outputcdr.hpp>
#include <acewrapper/inputcdr.hpp>

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

