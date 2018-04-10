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

#include "u5303a_global.hpp"
#include <AgMD2.h>
#include <boost/logic/tribool.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace acqrscontrols { namespace u5303a { class identify; class waveform; } }

namespace u5303a {

    class U5303ASHARED_EXPORT AgMD2 {
        class impl;
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif
        std::unique_ptr< impl > impl_;
#if defined _MSC_VER
#pragma warning(pop)
#endif
        ViSession session_;

        AgMD2( const AgMD2& ) = delete;
        AgMD2& operator = ( const AgMD2& ) = delete;

    public:
        ~AgMD2();
        AgMD2();

        inline ViSession session() { return session_; }

        static bool log( ViStatus rcode, const char * const file, int line
                         , std::function< std::string()> details = std::function<std::string()>() );

        uint32_t dataSerialNumber();
        
        ViStatus initWithOptions( const std::string& resource, ViBoolean idQuery, ViBoolean reset, const std::string& options );

        bool Identify( std::shared_ptr< acqrscontrols::u5303a::identify >& );
        std::shared_ptr< acqrscontrols::u5303a::identify > Identify();

        bool ConfigureTimeInterleavedChannelList( const std::string& channelName, const std::string& channelList );

        bool isSimulate() const;

        // bool setSampleRate( double sampleRate );

        // double SampleRate() const;

        bool setActiveTriggerSource( const std::string& trigSource );
        
        bool setTriggerCoupling( const std::string& trigSoruce, int32_t );
        int32_t TriggerCoupling( const std::string& trigSoruce ) const;
        
        bool setTriggerDelay( double );
        double TriggerDelay();

        bool setTriggerLevel( const std::string& trigSource, double );
        double TriggerLevel( const std::string& trigSource ) const;
        
        bool setTriggerSlope( const std::string& trigSource, int32_t slope );
        int32_t TriggerSlope( const std::string& trigSource ) const;  // 0:NEGATIVE, 1:POSITIvE
        
        bool setDataInversionEnabled( const std::string&, bool );

        bool setAcquisitionRecordSize( uint32_t );

        bool setAcquisitionNumberOfAverages( uint32_t );

        bool setAcquisitionNumRecordsToAcquire( uint32_t ); // MultiRecord

        bool setAcquisitionMode( int );
        int AcquisitionMode() const;

        bool CalibrationSelfCalibrate();

        bool AcquisitionInitiate();

        bool AcquisitionWaitForAcquisitionComplete( uint32_t milliseconds );
        bool isAcquisitionIdle() const;

        bool setTSREnabled( bool );
        bool TSREnabled();

        boost::tribool isTSRAcquisitionComplete() const;
        
        boost::tribool TSRMemoryOverflowOccured() const;

        bool TSRContinue();

        bool abort();

        bool setTriggerHoldOff( double seconds );

        double TriggerHoldOff() const;

        boost::tribool isIdle() const;
        boost::tribool isMeasuring() const;
        boost::tribool isWaitingForArm() const;
        boost::tribool isWaitingForTrigger() const;

        // for PKD+AVG POC purpose
        ViStatus setAttributeViInt32( ViConstString RepCapIdentifier, ViAttr AttributeID, ViInt32 AttributeValue );
        ViStatus getAttributeViInt32( ViConstString RepCapIdentifier, ViAttr AttributeID, int32_t& result ) const;
        
        ViStatus setAttributeViInt64( ViConstString RepCapIdentifier, ViAttr AttributeID, ViInt64 AttributeValue );

        ViStatus setAttributeViBoolean( ViConstString RepCapIdentifier, ViAttr AttributeID, ViBoolean AttributeValue );

        ViStatus getAttributeViString ( ViConstString RepCapIdentifier, ViAttr AttributeID, std::string& result ) const ;
        
        template< typename T > ViStatus setAttribute( ViConstString RepCapIdentifier, ViAttr AttributeID, T value );
        template< typename T > ViStatus getAttribute( ViConstString RepCapIdentifier, ViAttr AttributeID, T& value ) const;
        template< typename T > ViStatus setAttribute( ViAttr AttributeID, T value )        { return setAttribute( "", AttributeID, value ); }
        template< typename T > ViStatus getAttribute( ViAttr AttributeID, T& value ) const { return getAttribute( "", AttributeID, value ); }
    };

    template< ViAttr attr > struct attribute {
        AgMD2& _;
        attribute( AgMD2& t ) : _( t ) {}
        template< typename T > ViStatus set( const T& value ) { return _.setAttribute( attr, value ); }
        template< typename T > ViStatus get( T& value ) const { return _.getAttribute( attr, value ); }
        template< typename T > ViStatus set( ViConstString RepCapIdentifier, const T& value ) { return _.setAttribute( RepCapIdentifier, attr, value ); }
        template< typename T > ViStatus get( ViConstString RepCapIdentifier, T& value ) const { return _.getAttribute( RepCapIdentifier, attr, value ); }
        template< typename T > static ViStatus set( AgMD2& _, const T& value ) { return _.setAttribute( attr, value ); }
        template< typename T > static ViStatus get( AgMD2& _, T& value ) { return _.getAttribute( attr, value ); }
        template< typename T > static ViStatus set( AgMD2& _, ViConstString RepCapIdentifier, const T& value ) { return _.setAttribute( RepCapIdentifier, attr, value ); }
        template< typename T > static ViStatus get( AgMD2& _, ViConstString RepCapIdentifier, T& value ) { return _.getAttribute( RepCapIdentifier, attr, value ); }
    };
}
