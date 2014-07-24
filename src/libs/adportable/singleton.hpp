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

#pragma once

#include <atomic>
#include <mutex>

namespace adportable {

    std::atomic<T*> T::instance_ = 0;
    std::mutex T::mutex_;

    template<class T> class singleton {
    public:
        static T* instance() {
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
    };
}
