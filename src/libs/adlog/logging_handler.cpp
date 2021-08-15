/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "logging_syslog.hpp"
#include "logging_debug.hpp"
#include "logger.hpp"
#include <boost/format.hpp>
#include <fstream>
#if defined WIN32
#include <process.h>
#endif
#include <adportable/debug.hpp>

using namespace adlog;

std::mutex logging_handler::mutex_;

logging_handler::logging_handler()
{
#if defined WIN32
	pid_ = ::_getpid();
#else
    pid_ = ::getpid();
#endif
}

logging_handler *
logging_handler::instance()
{
    static logging_handler __instance;
    return &__instance;
}

boost::signals2::connection
logging_handler::register_handler( handler_type::slot_type subscriber )
{
    ADDEBUG() << "######################## register_handler ####################";
    return logger_.connect( subscriber );
}

void
logging_handler::appendLog( int pri
                            , const std::string& msg
                            , const std::string& file
                            , int line
                            , const std::chrono::system_clock::time_point& tp  )
{
    ADDEBUG() << "############### fire appendLog";
    logger_( pri, msg, file, line, tp );
}

void
logging_handler::close()
{
}
