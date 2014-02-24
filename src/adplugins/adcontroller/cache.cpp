// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "cache.hpp"
#include <adinterface/signalobserverS.h>
#include <mutex>
#include <algorithm>

namespace adcontroller {
    class CacheImpl {
    public:
        ~CacheImpl() {}
        CacheImpl() {}
        std::deque< SignalObserver::Observer_var > queue_;
    };
}

using namespace adcontroller;


Cache::~Cache()
{
}

Cache::Cache()
{
}

bool
Cache::write( long pos, SignalObserver::DataReadBuffer_var& rdbuf )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( fifo_.size() > CACHE_SIZE )
        fifo_.pop_front();
    fifo_.push_back( CacheItem( pos, rdbuf ) );
    return true;
}

bool
Cache::read( long pos, SignalObserver::DataReadBuffer_out rdbuf )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    
    if ( fifo_.empty() )
        return false;

    if ( pos < 0 ) {
        auto it = std::find_if( fifo_.rbegin(), fifo_.rend(), [&]( const CacheItem& item ){ return item.rdbuf_->fcn == 0; } );
        if ( it != fifo_.rend() ) {
            SignalObserver::DataReadBuffer_var buf( new SignalObserver::DataReadBuffer( it->rdbuf_ ) );
            rdbuf = buf._retn();
            return true;            
        }
        return false;
    }
    
    if ( ! fifo_.empty() && ( fifo_.front() <= pos && pos <= fifo_.back() ) ) {
        auto it = std::lower_bound( fifo_.begin(), fifo_.end(), pos );
        
        if ( it != fifo_.end() ) {
            SignalObserver::DataReadBuffer_var buf( new SignalObserver::DataReadBuffer( it->rdbuf_ ) );
            rdbuf = buf._retn();
            return true;
        }
    }

	return false;
}

void
Cache::uptime_range( unsigned long long& oldest, unsigned long long& newest )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( fifo_.empty() ) {
        oldest = newest = 0;
    } else {
        const Cache::CacheItem& first = fifo_.front();
        const Cache::CacheItem& last = fifo_.back();
        oldest = first.rdbuf_->uptime;
        newest = last.rdbuf_->uptime;
    }
} 

long
Cache::posFromTime( unsigned long long usec )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    auto it = std::lower_bound( fifo_.begin(), fifo_.end(), usec, []( const Cache::CacheItem& lhs, unsigned long long rhs ) {
            return lhs.rdbuf_->uptime < rhs;
        });
    if ( it != fifo_.end() )
        return it->pos_;
    return (-1);
}

//////////////////////////////////////////////////
Cache::CacheItem::CacheItem( long pos
                            , SignalObserver::DataReadBuffer_var& rb ) : pos_( pos )
                                                                       , rdbuf_( rb )
{
}

Cache::CacheItem::CacheItem( const CacheItem& t ) : pos_( t.pos_ )
                                                  , rdbuf_( t.rdbuf_ ) 
{
}

Cache::CacheItem::~CacheItem()
{
}
