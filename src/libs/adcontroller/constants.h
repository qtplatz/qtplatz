// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <ace/Message_Block.h>

namespace adcontroller {

    namespace constants {
 
        enum msg_type {
            MB_EVENTLOG = ACE_Message_Block::MB_USER
            , MB_INITIALIZE
            , MB_CONNECT
            , MB_TIME_VALUE
        };
        
        
    }
}
