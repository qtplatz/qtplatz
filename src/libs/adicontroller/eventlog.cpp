/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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

#include "eventlog.hpp"

using namespace adicontroller;

EventLog::EventLog()
{
}

EventLog::~EventLog()
{
}

EventLog::LogMessage::LogMessage() : logId_( 0 )
                                   , priority_( pri_DEBUG )
                                   , tv_( 0 )
{
}

uint32_t
EventLog::LogMessage::logId() const
{
    return logId_;
}

uint32_t
EventLog::LogMessage::priority() const
{
    return priority_;
}

uint64_t
EventLog::LogMessage::timeSinceEpock() const
{
    return tv_;
}

const char *
EventLog::LogMessage::msgId() const
{
    return msgId_.c_str();
}

const char *
EventLog::LogMessage::srcId() const
{
    return srcId_.c_str();
}

const char *
EventLog::LogMessage::format() const
{
    return format_.c_str();
}

const std::vector< std::string >&
EventLog::LogMessage::args() const
{
    return args_;
}

void
EventLog::LogMessage::setLogId( uint32_t v )
{
    logId_ = v;
}

void
EventLog::LogMessage::setPriority( uint32_t v )
{
    priority_ = v;
}

void
EventLog::LogMessage::setTimeSinceEpock( uint64_t v )
{
    tv_ = v;
}

// nanoseconds
void
EventLog::LogMessage::setMsgId( const std::string& v )
{
    msgId_ = v;
}

void
EventLog::LogMessage::setSrcId( const std::string& v )
{
    srcId_ = v;
}

void
EventLog::LogMessage::setFormat( const std::string& v )
{
    format_ = v;
}

void
EventLog::LogMessage::setArgs( const std::vector< std::string >& v )
{
    args_ = v;
}

