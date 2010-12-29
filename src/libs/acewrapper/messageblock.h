// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <boost/noncopyable.hpp>
#include <ace/Message_Block.h>

namespace acewrapper {

    // scoped_mblock_ptr : this keeps a pointer as an argument and make sure release and 
    // clear pointer to ensure no furthere refernce in the function.
    // This is not for smart_pointer.  For smart pointer purpose, should use boost::intrusive_ptr instead this.
    template<class MB=ACE_Message_Block> class scoped_mblock_ptr : boost::noncopyable {
        MB *& mb_;
    public:
        explicit scoped_mblock_ptr(MB *& mb) : mb_(mb) {}
        ~scoped_mblock_ptr() { dispose(); }
        operator MB * () { return mb_; }
        MB * operator->() { return mb_; }
    private:
        inline void dispose() {
            MB::release( mb_ );
            mb_ = 0;
        }
    };

    void intrusive_ptr_add_ref( ACE_Message_Block * ) { /* should NOT use neither duplicate nor clone */ }
    void intrusive_ptr_release( ACE_Message_Block * ptr ) { ACE_Message_Block::release(ptr); }

    class MessageBlock {
    public:
        MessageBlock();
    };
}
