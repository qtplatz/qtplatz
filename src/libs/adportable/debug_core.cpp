/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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
#include <fstream>
#include <boost/format.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

namespace adportable { namespace core {

        const char * share_mem_name = "adportable_debug_core";
    }
}

using namespace adportable::core;

debug_core * debug_core::instance_ = 0;
std::mutex debug_core::mutex_;

debug_core::~debug_core()
{
}

debug_core::debug_core() : logfname_( "debug.log" )
{
}

debug_core *
debug_core::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new debug_core();
    }
    return instance_;
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
    if ( file.empty() )
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

