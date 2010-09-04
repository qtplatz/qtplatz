// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

namespace device_emulator {
    namespace constants {

        enum msg_type {
			MB_EVENTLOG = ACE_Message_Block::MB_USER
            , MB_SENDTO_CONTROLLER
            , MB_HEARTBEAT_TO_CONTROLLER
            , MB_CLASS_TO_CONTROLLER
            , MB_DATA_TO_CONTROLLER
        };


    }
}
