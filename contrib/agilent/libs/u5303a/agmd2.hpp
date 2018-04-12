/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <boost/optional.hpp>
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

        //<------------------------  refactord code --------------------------
        ViStatus initWithOptions( const std::string& resource, ViBoolean idQuery, ViBoolean reset, const std::string& options );

        //------------------------  refactord code -------------------------->

        uint32_t dataSerialNumber();

        bool Identify( std::shared_ptr< acqrscontrols::u5303a::identify >& );

        std::shared_ptr< acqrscontrols::u5303a::identify > Identify();
        
        [[deprecated("replace with initWithOptions")]]
            bool InitWithOptions( const std::string& resource, ViBoolean idQuery, ViBoolean reset, const std::string& options );

        bool GetAttributeViString ( ViStatus&, ViConstString RepCapIdentifier, ViAttr AttributeID, std::string& result );
        bool GetAttributeViInt32( ViStatus&, ViConstString RepCapIdentifier, ViAttr AttributeID, int32_t& result );

        bool ConfigureTimeInterleavedChannelList( const std::string& channelName, const std::string& channelList );

        bool isSimulate() const;

        bool setSampleRate( double sampleRate );

        double SampleRate() const;

        bool setActiveTriggerSource( const std::string& trigSource );
        
        bool setTriggerCoupling( const std::string& trigSoruce, int32_t );
        int32_t TriggerCoupling( const std::string& trigSoruce ) const;
        
        bool setTriggerDelay( double );
        double TriggerDelay();

        bool setTriggerLevel( const std::string& trigSource, double );
        double TriggerLevel( const std::string& trigSource ) const;
        
        bool setTriggerSlope( const std::string& trigSource, int32_t slope );
        int32_t TriggerSlope( const std::string& trigSource ) const;  // 0:NEGATIVE, 1:POSITIvE

        [[deprecated("use attribute< data_inversion_enabled >::set")]] 
            bool setDataInversionEnabled( const std::string&, bool );

        [[deprecated("use attribute< record_size >::set")]] bool setAcquisitionRecordSize( uint32_t );

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
        ViStatus getAttributeViString( ViConstString RepCapIdentifier, ViAttr AttributeID, std::string& result ) const ;
        
        template< typename T > ViStatus setAttribute( ViConstString RepCapIdentifier, ViAttr AttributeID, T value );
        template< typename T > ViStatus getAttribute( ViConstString RepCapIdentifier, ViAttr AttributeID, T& value ) const;
    };

        // AGMD2_ATTR_SAMPLE_RATE
    struct acquisition_mode               { static constexpr ViAttr id = AGMD2_ATTR_ACQUISITION_MODE;               typedef ViInt32 value_type; };
    struct acquisition_number_of_averages { static constexpr ViAttr id = AGMD2_ATTR_ACQUISITION_NUMBER_OF_AVERAGES; typedef ViInt32 value_type; };
    struct active_trigger_source          { static constexpr ViAttr id = AGMD2_ATTR_ACTIVE_TRIGGER_SOURCE;          typedef std::string value_type; };
    struct channel_data_inversion_enabled { static constexpr ViAttr id = AGMD2_ATTR_CHANNEL_DATA_INVERSION_ENABLED; typedef bool value_type; };
    struct control_io_count               { static constexpr ViAttr id = AGMD2_ATTR_CONTROL_IO_COUNT;               typedef ViInt32 value_type; };
    struct instrument_info_nbr_adc_bits   { static constexpr ViAttr id = AGMD2_ATTR_INSTRUMENT_INFO_NBR_ADC_BITS;   typedef ViInt32 value_type; };
    struct is_idle                        { static constexpr ViAttr id = AGMD2_ATTR_IS_IDLE;                        typedef ViInt32 value_type; };
    struct num_records_to_acquire         { static constexpr ViAttr id = AGMD2_ATTR_NUM_RECORDS_TO_ACQUIRE;         typedef ViInt64 value_type; };
    struct peak_detection_amplitude_accumulation_enabled { static constexpr ViAttr id = AGMD2_ATTR_PEAK_DETECTION_AMPLITUDE_ACCUMULATION_ENABLED; typedef bool value_type; };
    struct peak_detection_falling_delta   { static constexpr ViAttr id = AGMD2_ATTR_PEAK_DETECTION_FALLING_DELTA;   typedef ViInt32 value_type; };
    struct peak_detection_rising_delta    { static constexpr ViAttr id = AGMD2_ATTR_PEAK_DETECTION_RISING_DELTA;    typedef ViInt32 value_type; };
    struct record_size                    { static constexpr ViAttr id = AGMD2_ATTR_RECORD_SIZE;                    typedef ViInt64 value_type; };
    struct sample_rate                    { static constexpr ViAttr id = AGMD2_ATTR_SAMPLE_RATE;                    typedef ViReal64 value_type; };
    struct simulate                       { static constexpr ViAttr id = AGMD2_ATTR_SIMULATE;                       typedef bool value_type; };
    struct trigger_coupling               { static constexpr ViAttr id = AGMD2_ATTR_TRIGGER_COUPLING;               typedef ViInt32 value_type;  };
    struct trigger_delay                  { static constexpr ViAttr id = AGMD2_ATTR_TRIGGER_DELAY;                  typedef ViReal64 value_type; };
    struct trigger_holdoff                { static constexpr ViAttr id = AGMD2_ATTR_TRIGGER_HOLDOFF;                typedef ViReal64 value_type; };
    struct trigger_level                  { static constexpr ViAttr id = AGMD2_ATTR_TRIGGER_LEVEL;                  typedef ViReal64 value_type; };
    struct trigger_slope                  { static constexpr ViAttr id = AGMD2_ATTR_TRIGGER_SLOPE;                  typedef ViInt32 value_type; };
    struct tsr_enabled                    { static constexpr ViAttr id = AGMD2_ATTR_TSR_ENABLED;                    typedef bool value_type; };
    struct tsr_is_acquisition_complete    { static constexpr ViAttr id = AGMD2_ATTR_TSR_IS_ACQUISITION_COMPLETE;    typedef bool value_type; };
    struct tsr_memory_overflow_occurred   { static constexpr ViAttr id = AGMD2_ATTR_TSR_MEMORY_OVERFLOW_OCCURRED;   typedef bool value_type; };

    //////////////////////////////////////////////////////
    struct agmd2_exception : std::exception { ViStatus rcode; agmd2_exception( ViStatus t ) : rcode( t ) {} };

    template< typename attribute_type > struct attribute {

        static ViStatus set( AgMD2& a, typename attribute_type::value_type const& value ) {
            return a.setAttribute( "", attribute_type::id, value );
        }
        
        template< typename T > static ViStatus get( AgMD2& a, T& value ) {
            return a.getAttribute( "", attribute_type::id, value );
        }

        static ViStatus set( AgMD2& a, ViConstString RepCapIdentifier, typename attribute_type::value_type const& value ) {
            return a.setAttribute( RepCapIdentifier, attribute_type::id, value );
        }
        
        static ViStatus get( AgMD2& a, ViConstString RepCapIdentifier, typename attribute_type::value_type& value ) {
            return a.getAttribute( RepCapIdentifier, attribute_type::id, value );
        }

        static boost::optional< typename attribute_type::value_type >
        value( AgMD2& a, ViStatus& rcode, ViConstString RepCapIdentifier = "" ) {
            typename attribute_type::value_type d;
            if ( ( rcode = a.getAttribute( RepCapIdentifier, attribute_type::id, d ) ) == VI_SUCCESS )
                return d;
            return boost::none;
        }        

        static typename attribute_type::value_type
        value( AgMD2& a, ViConstString RepCapIdentifier = "" ) {
            ViStatus rcode(0);
            typename attribute_type::value_type d;
            if ( ( rcode = a.getAttribute( RepCapIdentifier, attribute_type::id, d ) ) == VI_SUCCESS )
                return d;
            throw agmd2_exception( rcode );
        }        
    };

}
