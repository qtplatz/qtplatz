/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "logging_handler.hpp"
#include <adportable/profile.hpp>
#include <adportable/date_string.hpp>
#include <adportable/debug.hpp>
#include <adportable/debug_core.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>
#if defined WIN32
#include <process.h>
#endif
#if defined __linux__
# include <syslog.h>
#endif

using namespace adlog;

std::mutex logging_handler::mutex_;

static std::once_flag __flag;
static uint64_t __pid;

logging_handler::logging_handler()
{
    boost::filesystem::path logfile( adportable::profile::user_data_dir<char>() );
    logfile /= "adlog.log";
    logfile_ = logfile.string();
#if defined WIN32
	__pid = ::_getpid();
#else
    __pid = ::getpid();
#endif
#ifdef __linux__
    openlog( "adlog", LOG_CONS | LOG_PID,  LOG_USER );
#endif
    // adportable::core::debug_core::instance()->open( std::string() );  // disable file logging
}

logging_handler *
logging_handler::instance()
{
    static logging_handler __instance;
    return &__instance;
}

const std::string&
logging_handler::logfile() const
{
    return logfile_;
}

void
logging_handler::setlogfile( const std::string& logfile )
{
    logfile_ = logfile;
}

void
logging_handler::setpid( uint64_t pid )
{
    std::call_once( __flag, [&](){ __pid = pid; } );
}

boost::signals2::connection
logging_handler::register_handler( handler_type::slot_type subscriber )
{
    return logger_.connect( subscriber );
}

void
logging_handler::appendLog( int pri
                            , const std::string& msg
                            , const std::string& file
                            , int line
                            , const std::chrono::system_clock::time_point& tp  )
{
    // forward (for graphical logging display on qtplatz)
    logger_( pri, msg, file, line, tp );

    if ( !logfile_.empty() ) {
        std::ofstream of( logfile_.c_str(), std::ios_base::out | std::ios_base::app );
        of << adportable::date_string::logformat( tp ) << ":[" << __pid << "]\t" << msg << std::endl;
    }

    adportable::debug(file.c_str(),line) << adportable::date_string::logformat( tp ) << "\t" << msg;

#ifdef __linux__
    syslog( pri, "%s", msg.c_str() );
#endif
}

void
logging_handler::close()
{
}
