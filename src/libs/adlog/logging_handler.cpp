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

std::atomic<logging_handler * > logging_handler::instance_(0); // = 0
std::mutex logging_handler::mutex_;

logging_handler::logging_handler()
{
    boost::filesystem::path logfile( adportable::profile::user_data_dir<char>() );
    logfile /= "qtplatz.log";
    logfile_ = logfile.string();
}

logging_handler *
logging_handler::instance()
{
    typedef logging_handler T;

    T * tmp = instance_.load( std::memory_order_relaxed );
    std::atomic_thread_fence( std::memory_order_acquire );
    if ( tmp == nullptr ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        tmp = instance_.load( std::memory_order_relaxed );
        if ( tmp == nullptr ) {
            tmp = new T();
            std::atomic_thread_fence( std::memory_order_release );
            instance_.store( tmp, std::memory_order_relaxed );
        }
    }
    return tmp;
}

void
logging_handler::register_handler( handler_type f )
{
    loggers_.push_back( f );
}

logging_handler::iterator
logging_handler::begin()
{
    return loggers_.begin();
}

logging_handler::iterator
logging_handler::end()
{
    return loggers_.end();
}

size_t
logging_handler::size() const
{
    return loggers_.size();
}

void
logging_handler::appendLog( int pri, const std::string& msg, const std::string& file, int line
                            , const std::chrono::system_clock::time_point& tp  )
{
    for ( auto& client: *this )
        client( pri, msg, file, line, tp );
    
	std::ofstream of( logfile_.c_str(), std::ios_base::out | std::ios_base::app );
    of << adportable::date_string::logformat( tp ) << ":\t" << file << "(" << line << "): " << msg << std::endl;

    adportable::debug() << adportable::date_string::logformat( tp ) << ":\t" << file << "(" << line << "): " << msg;
}

void
logging_handler::close()
{
	loggers_.clear();
}

// static -- invoke from adportable::core::debug_core
void
logging_handler::log( int pri, const std::string& msg, const std::string& file, int line )
{
    auto tp = std::chrono::system_clock::now();
    for ( auto& client : *logging_handler::instance() )
        client( pri, msg, file, line, tp );
}

