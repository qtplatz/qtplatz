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

#include "eventlog_helper.hpp"
#include <ace/Time_Value.h>
#include "EventLogC.h"
#include <boost/format.hpp>
#include <sstream>

using namespace adinterface;
using namespace adinterface::EventLog;

std::wstring
adinterface::EventLog::LogMessageHelper::toString( const ::EventLog::LogMessage& msg )
{
    const wchar_t * fmt = msg.format.in();
    size_t narg = msg.args.length();

    boost::wformat format( fmt );

    for ( size_t i = 0; i < narg; ++i )
        format % static_cast<const wchar_t *>( msg.args[i].in() );

    std::wostringstream o;
    o << format;
    return o.str();
}

LogMessageHelper::LogMessageHelper( const std::wstring& format )
{
	msg_.tv.sec = time(0);
    msg_.tv.usec = 0;
    msg_.format = format.c_str();
}

LogMessageHelper::LogMessageHelper( const ACE_Time_Value& tv )
{
    msg_.tv.sec = tv.sec();
    msg_.tv.usec = tv.usec();
}

LogMessageHelper::LogMessageHelper( const LogMessageHelper& t ) : msg_( t.msg_ )
{
}

LogMessageHelper&
LogMessageHelper::format( const std::wstring& fmt )
{
    msg_.format = fmt.c_str();
    return *this;
}

