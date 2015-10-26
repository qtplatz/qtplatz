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

#include <AgMD2.h>
#include <string>
#include <memory>

namespace acqrscontrols { namespace u5303a { class identify; } }

namespace u5303a {

    class AgMD2 {

        ViSession session_;

        AgMD2( const AgMD2& ) = delete;
        AgMD2& operator = ( const AgMD2& ) = delete;

    public:
        ~AgMD2();
        AgMD2();


        inline ViSession session() { return session_; }

        static bool log( ViStatus rcode, const char * const file, int line );
        
        bool InitWithOptions( const std::string& resource, ViBoolean idQuery, ViBoolean reset, const std::string& options );

        bool GetAttributeViString ( ViStatus& rcode, ViConstString RepCapIdentifier, ViAttr AttributeID, std::string& result );

        bool Identify( acqrscontrols::u5303a::identify& ident );

        bool ConfigureTimeInterleavedChannelList( const std::string& channelName, const std::string& channelList );

        bool isSimulate() const;

        bool setSampleRate( double sampleRate );

        double SampleRate() const;

        bool setActiveTriggerSource( const std::string& trigSource );
        
        bool setTriggerCoupling( int32_t );
        bool setTriggerDelay( double );

        bool setTriggerLevel( const std::string& trigSource, double );
        double TriggerLevel( const std::string& trigSource ) const;
        
        bool setTriggerSlope( const std::string& trigSource, int32_t slope );
        int32_t TriggerSlope( const std::string& trigSource ) const;  // 0:NEGATIVE, 1:POSITIvE
        
        bool setDataInversionEnabled( bool );

        bool setAcquisitionRecordSize( uint32_t );

        bool setAcquisitionNumberOfAverages( uint32_t );

        bool setAcquisitionNumRecordsToAcquire( uint32_t ); // MultiRecord

        bool setAcquisitionMode( int );

        bool CalibrationSelfCalibrate();

        bool AcquisitionInitiate();

        bool AcquisitionWaitForAcquisitionComplete( uint32_t milliseconds );
        
    }; 
}
