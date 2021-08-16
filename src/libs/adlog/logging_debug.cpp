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

#include "logging_debug.hpp"
#include "logging_handler.hpp"
#include <adportable/date_string.hpp>
#include <adportable/debug.hpp>
#include <adportable/debug_core.hpp>

using namespace adlog;

logging_debug *
logging_debug::instance()
{
    static logging_debug __instance;
    return &__instance;
}

logging_debug::logging_debug()
{
}

logging_debug::~logging_debug()
{
    terminate();
}

void
logging_debug::operator()( int pri
                            , const std::string& msg
                            , const std::string& file
                            , int line
                            , const std::chrono::system_clock::time_point& tp ) const
{
    adportable::debug(file.c_str(),line) << adportable::date_string::logformat( tp ) << "\t" << msg;
}

bool
logging_debug::initialize()
{
    if ( ! connection_.connected() ) {
        connection_ = logging_handler::instance()->loggers().connect( *this );
    }
    return connection_.connected();
}

void
logging_debug::terminate()
{
    connection_.disconnect();
}
