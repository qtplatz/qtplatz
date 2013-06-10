// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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
#include <condition_variable>
#include <mutex>

namespace adportable {
    
    class barrier {
    public:
        barrier( unsigned int count ) : count_( count )
                                      , generation_( 0 )
                                      , nthread_( count ) {
            if ( count == 0 )
                throw std::exception( "barrier constractor: count cannot be zero." );
        }
        bool wait() {
            std::unique_lock< std::mutex > lock( mutex_ );
            unsigned int gen = generation_;
            if ( --count_ == 0 ) {
                generation_++;
                count_ = nthread_;
                cond_.notify_all();
                return true;
            }
            while( gen == generation_ )
                cond_.wait( lock );
            return false;
        }

    private:
        std::mutex mutex_;
        std::condition_variable cond_;
        unsigned int count_;
        unsigned int generation_;
        unsigned int nthread_;
    };
  
}
