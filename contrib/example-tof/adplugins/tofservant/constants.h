/**************************************************************************
** Copyright (C) 2012-2013 HORIBA STEC
** AUTHOR: Toshinobu Hondo, Ph.D.
** Derived from Science Liaison / Advanced Instrumentation Project
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

#include <ace/Message_Block.h>

namespace tofservant {

    namespace constants {

        enum msg_type {
            MB_EVENTLOG = ACE_Message_Block::MB_USER
            , MB_INITIALIZE
            , MB_CONNECT
            , MB_TIME_VALUE
            , MB_COMMAND
            , MB_SENDTO_DEVICE
            , MB_QUERY_DEVICE
            , MB_RECVFROM_DEVICE
            , MB_TOF_DATA
            , MB_TOF_ApplyMethod
            , MB_TOF_ApplyTuneMethod
            , MB_TOF_SetptChanged
            , MB_TOF_DATA_RF
        };

        enum SESSION_COMMAND {
            SESSION_SHELL
            , SESSION_INITIALIZE
            , SESSION_INIT_RUN
            , SESSION_START_RUN
            , SESSION_SUSPEND_RUN
            , SESSION_RESUME_RUN
            , SESSION_STOP_RUN  
            , SESSION_SHUTDOWN
            , SESSION_SENDTO_DEVICE  // subsequent DWORD should be class id
            , SESSION_QUERY_DEVICE   // subsequent DWORD should be class id
            , SESSION_ASYNC_COMMAND_BEGIN
            , SESSION_QUERY_DEVICE_DATA
            , SESSION_DEBUG_SAMPINTVAL
            , SESSION_DEBUG_PEAKWIDTH
        };

    }
}
