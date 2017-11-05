/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <boost/filesystem/path.hpp>
#include <fstream>

using namespace adlog;

std::mutex logging_handler::mutex_;

static std::once_flag __flag;
static uint64_t __pid;

logging_handler::logging_handler()
{
    boost::filesystem::path logfile( adportable::profile::user_data_dir<char>() );
    logfile /= "qtplatz.log";
    logfile_ = logfile.string();
    __pid = ::getpid();
}

logging_handler *
logging_handler::instance()
{
    static logging_handler __instance;
    return &__instance;
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
    
	std::ofstream of( logfile_.c_str(), std::ios_base::out | std::ios_base::app );
    of << adportable::date_string::logformat( tp ) << ":[" << __pid << "]\t" << msg << std::endl;

    adportable::debug(file.c_str(),line) << adportable::date_string::logformat( tp ) << "\t" << msg;
}

void
logging_handler::close()
{
}

