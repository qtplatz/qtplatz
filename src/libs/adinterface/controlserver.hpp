/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

namespace controlserver {

    enum eStatus { 
        eNothing                    // Not Initialized
        , eNotConfigured            //= 0x00000001,  // no instrument := no driver software loaded
        , eConfigured               //= 0x00000002,  // relevant software/driver has loaded
        , eInitializing             //= 0x00000003,  // startup initializing (only at the begining after startup)
        , eStandBy                  //= 0x00000004,  // instrument is stand by state
        , ePreparingForRun          //= 0x00000005,  // preparing for next method (parameters being be set value)
        , eReadyForRun              //= 0x00000006,  // method is in initial state, ready to run (INIT RUN, MS HTV is ready)
        , eWaitingForContactClosure //= 0x00000007,  //
        , eRunning                  //= 0x00000008,  // method is in progress
        , eStop                     //= 0x00000009,  // stop := detector is not monitoring, pump is off
    };
    
    enum eInstEvent  {
        event_Nothing,
        event_HeartBeat,
        event_InstStateChanged,
        event_ConfigChanged,
        event_UpdateData,
        event_StartIn,
        event_StartOut,
        event_InjectIn,
        event_InjectOut,
        event_EventIn,
        event_EventOut
    };

}

