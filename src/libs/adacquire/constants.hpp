/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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

namespace adacquire {

    namespace Instrument {
        
        enum eInstStatus {
            eNothing
            , eNotConnected             //= 0x00000001,  // no instrument := no driver software loaded
            , eOff                      //= 0x00000002,  // software driver can be controled, but hardware is currently off
            , eInitializing             //= 0x00000003,  // startup initializing (only at the begining after startup)
            , eStandBy                  //= 0x00000004,  // instrument is stand by state (Pump, detector lamps, HV etc. are on; method is indeterminant)
            , ePreparingForRun          //= 0x00000005,  // preparing for next method (parameters being be set value)
            , eReadyForRun              //= 0x00000006,  // method is in initial state, ready to run (INIT RUN, MS HTV is ready)
            , eWaitingForContactClosure //= 0x00000007,  //
            , eRunning                  //= 0x00000008,  // method is in progress
            , eStop                     //= 0x00000009,  // stop := detector is not monitoring, pump is off
            , eErrorFlag = 0x80000000
        };

        enum eInstEvent  {
            instEventNothing,
            instEventHeartBeat,
            instEventStateChanged,
            instEventConfigChanged,
            instEventUpdateData,
            instEventStartIn,
            instEventStartOut,
            instEventInjectIn,
            instEventInjectOut,
            instEventEventIn,
            instEventEventOut
        };

        enum idFSMAction {
            fsmStop
            , fsmStart    // --> PreparingForRun
            , fsmReady    // --> WaitForContactClosure
            , fsmInject   // --> Running
            , fsmComplete // --> Dormant
        };

    };

    namespace SignalObserver {

        enum wkEvent {
            wkEvent_Error           = 0x80000000
            , wkEvent_Warning         = 0x40000000  // instrument is warning state
            , wkEvent_Marker          = 0x20000000  // wiring to 'marker trigger in'
            , wkEvent_INJECT          = 0x10000000  // wiring to 'inject trigger in'
            , wkEvent_MethodStart     = 0x08000000  // wiring to 'method start in'
            , wkEvent_DataWarning     = 0x04000000  // data waring such as input over range.
            , wkEvent_DarkInProgress  = 0x02000000  // dark signal acquiring
            , wkEvent_AcqInProgress   = 0x01000000  // Data storing, INJ trigger disarmed
            , wkEvent_UserEventsMask  = 0x00ffffff  // wiring to 'event in/out' terminal box
        };

        enum eTRACE_METHOD {
            eTRACE_TRACE
            , eTRACE_SPECTRA
            , eDIAGNOSTIC  // events, LC flow/pressure profile, column oven temp profile etc.
            , eTRACE_IMAGE_TDC       // Series of 2-D image frames, in time domain (raw data from TDC array, such as MALPIX)
            , eTRACE_IMAGE_SPECTRA   // Serial of 2-D (surface giometric) imaging spectra
            , eTRACE_IMAGE_INTENSITY // Serias of 2-D image frames, in intensity domain, such as total ion count image map
            , eTRACE_TDC_1D
            , eTRACE_TDC_HISTOGRAM
        };

        enum eSPECTROMETER {
            eUnknownSpectrometer
            , eMassSpectrometer
            , eUVSpectrometer
            , eCDSpectrometer
            , eIRSpectrometer
            , eRamanSpectrometer
            , eFluorescenceSpectrometer 
        };

        enum eUpdateFrequency {
            Realtime
            , Frequent
            , Sometimes
            , HalfFull
            , WellKnownEventsOnly
        };

        enum eConfigStatus {
            Offline
            , Online
        };
    }
    
} // namespace adicontroler
