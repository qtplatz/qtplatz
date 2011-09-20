/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "logger.hpp"
#include "manager_i.hpp"

using namespace adcontroller;

Logger::Logger( const std::wstring& format
                , ::EventLog::eMSGPRIORITY pri
                , const std::wstring& srcId
                , const std::wstring& msgId ) : msg( format, pri, srcId, msgId )
{
}

Logger::~Logger()
{
    commit();
}

void
Logger::commit()
{
    // Broker::EventLog, that is not EventLog
    Broker::Logger_var logger = static_cast< manager_i *>(*manager_i::instance())->getLogger();
    if ( ! CORBA::is_nil( logger ) ) {
        // logger->log( msg.get() );
    }
}

// void
// Logger::operator()( long pri, long msgId, const std::wstring& msg )
// {
//     (void)msgId;
//     TOFTask * task = infitofd::manager::instance()->task();
//     if ( task ) {
//         std::vector< std::wstring > args;
//         task->session_fire_log( pri, msg, args );
//     }
//     // TODO;
//     // obtin Broker::Manager_var from broker_manager_ior_;
//     // get Logger_var from Broker::Manager::getLogger()
//     // do  Logger::log( EventLog& )
// }

