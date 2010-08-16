// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "eventlog_helper.h"
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

