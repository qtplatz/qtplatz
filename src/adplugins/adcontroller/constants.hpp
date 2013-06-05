// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include <ace/Message_Block.h>

namespace adcontroller {

    namespace constants {
 
        enum msg_type {
            MB_EVENTLOG = ACE_Message_Block::MB_USER
            , MB_INITIALIZE
            , MB_CONNECT
            , MB_TIME_VALUE
            , MB_MESSAGE
            , MB_COMMAND
            , MB_OBSERVER_UPDATE_DATA    // objid, pos
            , MB_OBSERVER_UPDATE_METHOD  // objid, pos
            , MB_OBSERVER_UPDATE_EVENT   // objid, pos, event 
        };

        enum commands {
            SESSION_COMMAND_ECHO = 0x100
            , SESSION_COMMAND_INITRUN
            , SESSION_COMMAND_STARTRUN
            , SESSION_COMMAND_STOPRUN
        };
        
    }
}
