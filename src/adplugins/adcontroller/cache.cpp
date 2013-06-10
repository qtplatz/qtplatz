// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
# include "cache.hpp"
# include <adinterface/signalobserverS.h>
# include <ace/Recursive_Thread_Mutex.h>

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
    acewrapper::scoped_mutex_t<> lock( mutex_ );

    if ( fifo_.size() > 1024 )
        fifo_.pop_front();
    fifo_.push_back( CacheItem( pos, rdbuf ) );
    return true;
}

bool
Cache::read( long pos, SignalObserver::DataReadBuffer_out rdbuf )
{
    acewrapper::scoped_mutex_t<> lock( mutex_ );
    
    if ( fifo_.empty() )
        return false;

    if ( pos < 0 )
        pos = fifo_.back();
    
    if ( ! fifo_.empty() && ( fifo_.front() <= pos && pos <= fifo_.back() ) ) {
        std::deque< CacheItem >::iterator it = std::lower_bound( fifo_.begin(), fifo_.end(), pos );
        
        if ( it != fifo_.end() ) {
            SignalObserver::DataReadBuffer_var buf( new SignalObserver::DataReadBuffer( it->rdbuf_ ) );
            rdbuf = buf._retn();
            return true;
        }
    }
	return false;
}

namespace adcontroller {
    namespace cache {
        struct time_compare {
            bool operator () ( const Cache::CacheItem& lhs, unsigned long long rhs ) const {
                return lhs.rdbuf_->uptime < rhs;
            }
            bool operator () ( unsigned long long lhs, const Cache::CacheItem& rhs ) const {
                return lhs < rhs.rdbuf_->uptime;
            }
            bool operator () ( const Cache::CacheItem& lhs, const Cache::CacheItem& rhs ) const {
                return lhs.rdbuf_->uptime < rhs.rdbuf_->uptime;
            }
        };
    } // cache
} // adcontroller

void
Cache::uptime_range( unsigned long long& oldest, unsigned long long& newest )
{
    acewrapper::scoped_mutex_t<> lock( mutex_ );

    if ( fifo_.empty() ) {
        oldest = newest = 0;
    } else {
        const Cache::CacheItem& first = fifo_.front();
        const Cache::CacheItem& last = fifo_.back();
        oldest = first.rdbuf_->uptime;
        newest = last.rdbuf_->uptime;
    }
    // std::cerr << "Cache::uptime_range return " << oldest << ", " << newest << std::endl;    
} 

long
Cache::posFromTime( unsigned long long usec )
{
    using adcontroller::cache::time_compare;

    acewrapper::scoped_mutex_t<> lock( mutex_ );

    std::deque< CacheItem >::iterator it = std::lower_bound( fifo_.begin(), fifo_.end(), usec, time_compare() );    
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
