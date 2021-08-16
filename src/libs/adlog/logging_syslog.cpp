/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "logging_syslog.hpp"
#include "logging_handler.hpp"
#include <boost/format.hpp>
#ifdef __linux__
# include <syslog.h>
#endif
#include <adportable/debug.hpp>

using namespace adlog;

logging_syslog *
logging_syslog::instance()
{
    static logging_syslog __instance;
    return &__instance;
}

logging_syslog::logging_syslog()
{
#ifdef __linux__
    // openlog( boost::log::attributes::current_process_name().get().c_str(), LOG_CONS | LOG_PID,  LOG_USER );
    openlog( nullptr, LOG_CONS | LOG_PID,  LOG_USER );
#endif
}

logging_syslog::~logging_syslog()
{
#ifdef __linux__
    closelog();
    terminate();
#endif
}

void
logging_syslog::operator()( int pri
                            , const std::string& msg
                            , const std::string& file
                            , int line
                            , const std::chrono::system_clock::time_point& ) const
{
#ifdef __linux__
    syslog( pri, "%s", ( boost::format( "%s; at %s(%d)" ) % msg % file % line ).str().c_str() );
#endif
}

bool
logging_syslog::initialize()
{
    if ( ! connection_.connected() ) {
        connection_ = logging_handler::instance()->register_handler( *this );
    }
    return connection_.connected();
}

void
logging_syslog::terminate()
{
    connection_.disconnect();
}
