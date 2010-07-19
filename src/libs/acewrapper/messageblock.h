// This is a -*- C++ -*- header.
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
