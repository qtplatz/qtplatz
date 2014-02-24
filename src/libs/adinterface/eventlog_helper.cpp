// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "eventlog_helper.hpp"
#include "eventlogC.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <sstream>
#include <chrono>

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
    CORBA::ULong narg = msg.args.length();

    boost::wformat format( fmt );

    for ( CORBA::ULong i = 0; i < narg; ++i )
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
	msg_.priority = pri;
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	msg_.tv.sec = time_t( std::chrono::duration_cast< std::chrono::seconds >(duration).count() );
	msg_.tv.usec = long( std::chrono::duration_cast< std::chrono::microseconds >( duration ).count() - (msg_.tv.sec * 1000000) );

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

