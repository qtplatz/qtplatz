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
#include <ace/OS_NS_sys_time.h>
#include "eventlogC.h"
#include <boost/format.hpp>
#include <sstream>

namespace adinterface { namespace EventLog {

        template<> LogMessageHelper& LogMessageHelper::operator % (const std::string& t)
        {
            msg_.args.length( msg_.args.length() + 1 ); // add an argument in vector
            std::wstring ws;
            ws.assign( t.begin(), t.end() );
            msg_.args[ msg_.args.length() - 1 ] = CORBA::wstring_dup( ws.c_str() );
            return *this;
        }
        
    }
}

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
    try {
        o << format;
    } catch ( boost::exception& ) {
        return fmt;
    }
    return o.str();
}

LogMessageHelper::LogMessageHelper( const std::wstring& format
                                   , unsigned long pri
                                    , const std::wstring& msgId
                                    , const std::wstring& srcId )
{
    ACE_Time_Value tv( ACE_OS::gettimeofday() );
    msg_.tv.sec = tv.sec();
    msg_.tv.usec = tv.usec();

    msg_.priority = pri;

    if ( ! msgId.empty() )
        msg_.msgId = CORBA::wstring_dup( msgId.c_str() );
    if ( ! srcId.empty() )
        msg_.srcId = CORBA::wstring_dup( srcId.c_str() );
    if ( ! format.empty() )
        msg_.format = CORBA::wstring_dup( format.c_str() );
}

LogMessageHelper::LogMessageHelper( const LogMessageHelper& t ) : msg_( t.msg_ )
{
}

LogMessageHelper&
LogMessageHelper::format( const std::wstring& fmt )
{
    msg_.format = CORBA::wstring_dup( fmt.c_str() );
    return *this;
}

