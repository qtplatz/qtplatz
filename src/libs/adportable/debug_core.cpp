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

#include "debug_core.hpp"
#include <iostream>
#include <fstream>
#include <boost/format.hpp>
#include "profile.hpp"

using namespace adportable::core;

std::atomic< debug_core * > debug_core::instance_ = 0;
std::mutex debug_core::mutex_;

debug_core::~debug_core()
{
}

debug_core::debug_core() : logfname_( profile::user_data_dir<char>() + "/debug.log" )
{
}

debug_core *
debug_core::instance()
{
    typedef debug_core T;

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

const std::string&
debug_core::logfile() const
{
    return logfname_;
}

bool
debug_core::open( const std::string& logfile )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    logfname_ = logfile;
    return true;
}

void
debug_core::log( int pri, const std::string& msg, const std::string& file, int line ) const
{
    if ( ! hooks_.empty() ) {
        std::lock_guard< std::mutex > lock( mutex_ );    
        for ( auto& logger: hooks_ )
            logger( pri, msg, file, line );
    }

    std::string loc;
    if ( !file.empty() )
        loc = ( boost::format( "%1%(%2%): " ) % file % line ).str();
    
    if ( !logfname_.empty() ) {
        std::ofstream of( logfname_.c_str(), std::ios_base::out | std::ios_base::app );
        of << loc << msg << std::endl;
    }
    std::cout << loc << msg << std::endl;
}

void
debug_core::hook( hook_handler_type f )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    hooks_.push_back( f );
}

void
debug_core::unhook()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    hooks_.clear();
}

