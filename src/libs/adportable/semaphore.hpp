/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <mutex>
#include <condition_variable>

namespace adportable {

    class semaphore {
        semaphore( const semaphore& ) = delete;
        semaphore& operator = ( const semaphore& ) = delete;
        
        size_t count_;
        std::mutex mutex_;
        std::condition_variable condition_;
        
    public:
        explicit semaphore( size_t ini = 0 ) : count_( ini ) {
        }

        inline size_t count() const { return count_; }

        // V
        inline void signal() {
            std::lock_guard< std::mutex > lock( mutex_ );
            ++count_;
            condition_.notify_one();
        }

        // P
        inline void wait() {
            std::unique_lock< std::mutex > lock( mutex_ );
            condition_.wait( lock, [=]{ return count_ > 0; } );
            --count_;
        }

        inline bool try_wait() {
            std::unique_lock< std::mutex > lock( mutex_ );
            if ( count_ ) {
                --count_;
                return true;
            }
            return false;
        }

        // C++ BasicLocable
        void lock() { wait(); }
        void unlock() { signal(); }
        // C++ Locable
        bool try_lock() { return try_wait(); }        
    };
}



