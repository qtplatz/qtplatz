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

namespace tofcontroller {

    namespace constants {

        enum msg_type {
			MB_EVENTLOG = ACE_Message_Block::MB_USER
            , MB_INITIALIZE
            , MB_CONNECT
            , MB_TIME_VALUE
			, MB_COMMAND
            , MB_DEBUG
			, MB_MCAST
			, MB_DGRAM
			, MB_SENDTO_DEVICE
            , MB_QUERY_DEVICE
			, MB_RECVFROM_DEVICE
        };

        enum SESSION_COMMAND {
            SESSION_INITIALIZE
            , SESSION_PREPARE_FOR_RUN 
            , SESSION_START_RUN
            , SESSION_SUSPEND_RUN
            , SESSION_RESUME_RUN
            , SESSION_STOP_RUN  
			, SESSION_SHUTDOWN
            , SESSION_SENDTO_DEVICE  // subsequent DWORD should be class id
			, SESSION_QUERY_DEVICE   // subsequent DWORD should be class id
            , SESSION_REMOTE_TRIGGER
            , SESSION_ARM_TRIGGER
            , SESSION_DISARM_TRIGGER
        };

    }
}
