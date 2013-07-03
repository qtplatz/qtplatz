/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#ifndef RAWXFER_HPP
#define RAWXFER_HPP

#include "dma_type.hpp"
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/array.hpp>
#include <iostream>

namespace tofinterface {

    class rawxfer {
    public:

        ~rawxfer();
        rawxfer();
        const char * share_name();
        const char * mutex_name();

        bool sbrk( std::size_t octets = 1024 * 1204 ); // 1Mo by default

        template<class dma_type> dma_type * allocate( const char * object_name ) {
            try {
                dma_type * dma = shm_->find_or_construct< dma_type >( object_name )();
                return dma;
            } catch ( std::exception& ex )  {
                std::cout << ex.what();
                return 0;
            }
        }

        template<class dma_type> void destroy(  const char * object_name ) {
            dma_type * ptr = find<dma_type>( object_name );
            if ( ptr )
                shm_->destroy_ptr( ptr );
        }

        boost::interprocess::interprocess_mutex& mutex() { return *mutex_; }

        template<class dma_type> dma_type * find( const char * object_name ) {
            std::pair< dma_type *, std::size_t> r = shm_->find< dma_type >( object_name );
            return r.first;
        }

        inline operator bool () const { return shm_ != 0; }

    private:
        boost::interprocess::managed_shared_memory * shm_;
        boost::interprocess::interprocess_mutex * mutex_;
    };

}

#endif // RAWXFER_HPP
