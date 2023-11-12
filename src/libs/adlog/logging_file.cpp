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

#include "logging_file.hpp"
#include "logging_handler.hpp"
#include <adportable/profile.hpp>
#include <adportable/profile.hpp>
#include <adportable/date_string.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <filesystem>
#include <fstream>

using namespace adlog;

logging_file *
logging_file::instance()
{
    static logging_file __instance;
    return &__instance;
}

logging_file::logging_file()
{
    std::error_code ec;
    auto program = std::filesystem::path( boost::dll::program_location( ec ).string() );
    logfile_ =
        ( std::filesystem::path( adportable::profile::user_data_dir<char>() )
          / program.stem() ).replace_extension(".log").string();
}

logging_file::~logging_file()
{
    connection_.disconnect();
}

void
logging_file::operator()( int pri
                            , const std::string& msg
                            , const std::string& file
                            , int line
                            , const std::chrono::system_clock::time_point& tp ) const
{
    if ( !logfile_.empty() ) {
        std::ofstream of( logfile_.c_str(), std::ios_base::out | std::ios_base::app );
        of << adportable::date_string::logformat( tp ) << ":[" << logging_handler::instance()->pid() << "]\t" << msg << std::endl;
    }
}

bool
logging_file::initialize()
{
    connection_ = logging_handler::instance()->register_handler( *this );
    return true;
}

void
logging_file::terminate()
{
    connection_.disconnect();
}

void
logging_file::destroy()
{
    delete this;
}
