// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

class ACE_Recursive_Thread_Mutex;

namespace adportable {

    class Cache {
    public:
        ~Cache();
        Cache( size_t itemsize, size_t nitems );
        unsigned long * rd_ptr();
        unsigned long * wr_ptr();
        unsigned long * rd_ptr( unsigned long& npos );
        void unlock( unsigned long * );
        struct ctrl_block {
            unsigned long flag_;
            unsigned long size_;
            unsigned long mblk_hdr_[2];
        };
        
        inline size_t capacity() const { 
            return (item_size_ * sizeof(unsigned long)) - sizeof(ctrl_block) + sizeof(unsigned long);
        };
        
        inline bool is_valid() const {
            return pool_ && pool_[0];
        };
        inline unsigned long rd_count() const { return rd_count_; }
        inline unsigned long wr_count() const { return wr_count_; }
        void reset();

    private:
        const size_t item_size_; // 256 * 1024 
        const size_t nitems_;    // 512
        unsigned long ** pool_;

        enum { eWriteBusy = 1, eWriteComplete = 2, eReadBusy = 4, eFree = 8 };

        unsigned long wr_count_;
        unsigned long rd_count_;
        
        void dispose();

        ACE_Recursive_Thread_Mutex * lock_;
    };

}
