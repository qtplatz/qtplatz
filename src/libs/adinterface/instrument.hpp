/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

namespace adinterface {

    namespace instrument {
    
        enum eInstStatus {
            eNothing
            , eNotConnected             //= 0x00000001,  // no instrument := no driver software loaded
            , eOff                      //= 0x00000002,  // software driver can be controled, but hardware is currently off
            , eInitializing             //= 0x00000003,  // startup initializing (only at the begining after startup)
            , eStandBy                  //= 0x00000004,  // instrument is stand by state
            , ePreparingForRun          //= 0x00000005,  // preparing for next method (parameters being be set value)
            , eReadyForRun              //= 0x00000006,  // method is in initial state, ready to run (INIT RUN, MS HTV is ready)
            , eWaitingForContactClosure //= 0x00000007,  //
            , eRunning                  //= 0x00000008,  // method is in progress
            , eStop                     //= 0x00000009,  // stop := detector is not monitoring, pump is off
            , eError
        };

    }
}
