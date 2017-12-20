/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "writeaccess.hpp"
#include <adportable/bzip2.hpp>

using namespace adacquire;
using namespace adacquire::SignalObserver;

        
WriteAccess::WriteAccess()
{
}

size_t
WriteAccess::operator << ( std::shared_ptr< const DataReadBuffer >&& p )
{
    vec_.emplace_back( p );
    return vec_.size();
}

void
WriteAccess::rewind()
{
    it_ = vec_.begin();
}

bool
WriteAccess::next()
{
    return ++it_ != vec_.end();
}

size_t
WriteAccess::ndata() const
{
    return vec_.size();
}

uint64_t
WriteAccess::elapsed_time() const
{
    return (*it_)->elapsed_time();
}

uint64_t
WriteAccess::epoch_time() const
{
    return (*it_)->epoch_time();
}

uint64_t
WriteAccess::pos() const
{
    return (*it_)->pos();
}

uint32_t
WriteAccess::fcn() const
{
    return (*it_)->fcn();
}

uint32_t
WriteAccess::events() const
{
    return (*it_)->events();
}

size_t
WriteAccess::xdata( std::string& ar ) const
{
    adportable::bzip2::compress( ar, reinterpret_cast<const char *>( (*it_)->xdata().data() ), (*it_)->xdata().size() );
    return ar.size();
}

size_t
WriteAccess::xmeta( std::string& ar ) const
{
    adportable::bzip2::compress( ar, reinterpret_cast<const char *>( (*it_)->xmeta().data() ), (*it_)->xmeta().size() );
    return ar.size();    
}


