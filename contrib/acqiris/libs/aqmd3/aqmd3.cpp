/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "aqmd3.hpp"
#include "error_message.hpp"
#include <aqmd3controls/identify.hpp>
#include <aqmd3controls/waveform.hpp>
#include <adportable/debug.hpp>
#include <adlog/logger.hpp>
#include <boost/format.hpp>
#include <boost/logic/tribool.hpp>
#include <atomic>

namespace aqmd3 {

    struct timeStampReader {
        ViInt64 markerArraySize;
        ViInt64 const timestampSize = 16;
        std::vector< ViInt32 > markerArray;
        timeStampReader() : markerArraySize(0) {}

        uint64_t operator()( aqmd3::AqMD3& md3 ) {
            ViInt64 addressLow = 0xFF800000;
            ViInt32 addressHigh_Ch1 = 0x00000080; // To read the Peak Histogram on CH1
            ViInt64 actualPoints, firstValidPoint;

            md3.QueryMinWaveformMemory(32, 1, 0, timestampSize, markerArraySize );
            markerArray.resize( markerArraySize );

            md3.LogicDeviceReadIndirectInt32( "DpuA"
                                               , addressHigh_Ch1
                                               , addressLow
                                               , timestampSize
                                               , markerArraySize
                                               , markerArray.data()
                                               , actualPoints
                                               , firstValidPoint );
            // ADDEBUG() << "TS :" << markerArray[ firstValidPoint + 2 ] << ":" << markerArray[ firstValidPoint + 1 ] << ", firstValidPoint: " << firstValidPoint;
            ViUInt64 value = uint64_t(markerArray[firstValidPoint + 1]) + (uint64_t(markerArray[firstValidPoint + 2]) << 32);
            return value;
        }
    };


    class AqMD3::impl {

        std::atomic< uint32_t > dataSerialNumber_;
    public:
        std::shared_ptr< aqmd3controls::identify > ident_;
        std::unique_ptr< timeStampReader > tsReader_;

    public:
        impl() : dataSerialNumber_( 0 )
               , tsReader_( std::make_unique< timeStampReader >() ) {
        }

        inline uint32_t dataSerialNumber() {
            return dataSerialNumber_++;
        }
    };

}

using namespace aqmd3;

AqMD3::AqMD3() : impl_( new impl() )
               , session_( -1 )
{
}

AqMD3::~AqMD3()
{
    //AqMD3_close( session_ );
}

ViStatus
AqMD3::initWithOptions( const std::string& resource, ViBoolean idQuery, ViBoolean reset, const std::string& options )
{
    return AqMD3_InitWithOptions( const_cast< char *>(resource.c_str())
                                  , idQuery
                                  , reset
                                  , options.c_str()
                                  , &session_ );
}

void
AqMD3::syslog( ViStatus rcode, const char * const file, int line, std::function< std::string()> describe ) const
{
    if ( rcode ) {
        ViChar msg[256] = {0};
        ViStatus errCode;
        AqMD3_GetError( session_, &errCode, sizeof(msg), msg );
        adlog::logger( file, line ) << boost::format("0x%x: %s where %s") % errCode % msg % ( describe ? describe() : "" );
        adportable::debug( file, line ) << boost::format("0x%x: %s where %s") % errCode % msg % ( describe ? describe() : "" );
    }
}

bool
AqMD3::clog( ViStatus rcode, const char * const file, int line, std::function< std::string()> describe ) const
{
    if ( rcode != VI_SUCCESS ) {
        if ( session_ == (-1) ) {
            auto format = describe ? boost::format( "0x%x: %s where %s" ) : boost::format( "0x%x: %s%s" );
            adportable::debug( file, line ) << format % rcode % error_message()[ rcode ] % ( describe ? describe() : "" );
        } else {
            ViStatus errCode;
            ViChar msg[256] = {0};
            auto format = describe ? boost::format( "0x%x/0x%x: %s where %s" ) : boost::format( "0x%x/0x%x: %s%s" );
            AqMD3_GetError( session_, &errCode, sizeof(msg), msg );
            adportable::debug( file, line ) << format % rcode % errCode % msg % ( describe ? describe() : "" );
        }
    }
    return rcode == VI_SUCCESS;
}

bool
AqMD3::GetAttributeViString ( ViStatus& rcode, ViConstString RepCapIdentifier, ViAttr AttributeID, std::string& result )
{
    ViChar str[128];

    result.clear();
    if ( ( rcode = AqMD3_GetAttributeViString( session_, RepCapIdentifier, AttributeID, sizeof( str ), str ) ) == VI_SUCCESS ) {
        result = str;
        return true;
    }

    clog( rcode, __FILE__, __LINE__ );
    return rcode == VI_SUCCESS;
}

bool
AqMD3::GetAttributeViInt32 ( ViStatus& rcode, ViConstString RepCapIdentifier, ViAttr AttributeID, int32_t& result )
{
    if ( ( rcode = AqMD3_GetAttributeViInt32( session_, RepCapIdentifier, AttributeID, reinterpret_cast<ViInt32 *>( &result ) ) ) == VI_SUCCESS )
         return true;

    clog( rcode, __FILE__, __LINE__ );
    return rcode == VI_SUCCESS;
}

bool
AqMD3::Identify( std::shared_ptr< aqmd3controls::identify > ident )
{
    ViStatus rcode(0);
    std::string str;

    if ( GetAttributeViString( rcode, "", AQMD3_ATTR_SPECIFIC_DRIVER_PREFIX, str ) )
        ident->Identifier() = str;

    if ( GetAttributeViString( rcode, "", AQMD3_ATTR_SPECIFIC_DRIVER_REVISION, str ) )
        ident->Revision() = str;

    if ( GetAttributeViString( rcode, "", AQMD3_ATTR_SPECIFIC_DRIVER_VENDOR, str ) )
        ident->Vendor() = str;

    if ( GetAttributeViString( rcode, "", AQMD3_ATTR_SPECIFIC_DRIVER_DESCRIPTION, str ) )
        ident->Description() = str;

    if ( GetAttributeViString( rcode, "", AQMD3_ATTR_INSTRUMENT_MODEL, str) )
        ident->InstrumentModel() = str;

    if ( GetAttributeViString( rcode, "", AQMD3_ATTR_INSTRUMENT_INFO_OPTIONS, str) )
        ident->Options() = str;

    if ( GetAttributeViString( rcode, "", AQMD3_ATTR_INSTRUMENT_FIRMWARE_REVISION, str ) )
        ident->FirmwareRevision() = str;

    if ( GetAttributeViString( rcode, "", AQMD3_ATTR_INSTRUMENT_INFO_SERIAL_NUMBER_STRING, str ) )
        ident->SerialNumber() = str;

    if ( GetAttributeViString( rcode, "", AQMD3_ATTR_INSTRUMENT_INFO_IO_VERSION, str ) )
        ident->IOVersion() = str;

    int32_t nbrADCBits(0);
    if ( AqMD3_GetAttributeViInt32( rcode, "", AQMD3_ATTR_INSTRUMENT_INFO_NBR_ADC_BITS, reinterpret_cast<ViInt32 *>(&nbrADCBits) ) )
        ident->NbrADCBits() = nbrADCBits;

    impl_->ident_ = ident;

    return true;
}

std::shared_ptr< aqmd3controls::identify >
AqMD3::Identify()
{
    return impl_->ident_;
}

uint32_t
AqMD3::dataSerialNumber()
{
    return impl_->dataSerialNumber();
}

bool
AqMD3::ConfigureTimeInterleavedChannelList( const std::string& channelName, const std::string& channelList )
{
    return clog( AqMD3_ConfigureTimeInterleavedChannelList( session_, channelName.c_str(), channelList.c_str() ), __FILE__, __LINE__ );
}

bool
AqMD3::ConfigureChannel( const std::string& channelName, ViReal64 range, ViReal64 offset, ViInt32 coupling, ViBoolean enable )
{
    return clog( AqMD3_ConfigureChannel( session_, channelName.c_str(), range, offset, coupling, enable ), __FILE__, __LINE__ );
}

bool
AqMD3::isSimulate() const
{
    ViBoolean simulate(false);
    return clog( AqMD3_GetAttributeViBoolean( session_, "", AQMD3_ATTR_SIMULATE, &simulate ), __FILE__, __LINE__ );
}

// Added for PKD+AVG POC quick test
ViStatus
AqMD3::setAttributeViInt32( ViConstString RepCapIdentifier, ViAttr AttributeID, ViInt32 AttributeValue )
{
    return AqMD3_SetAttributeViInt32( session_, RepCapIdentifier, AttributeID, AttributeValue );
}

ViStatus
AqMD3::setAttributeViInt64( ViConstString RepCapIdentifier, ViAttr AttributeID, ViInt64 AttributeValue )
{
    return AqMD3_SetAttributeViInt32( session_, RepCapIdentifier, AttributeID, AttributeValue );
}

ViStatus
AqMD3::setAttributeViBoolean( ViConstString RepCapIdentifier, ViAttr AttributeID, ViBoolean AttributeValue )
{
    return AqMD3_SetAttributeViBoolean( session_, RepCapIdentifier, AttributeID, AttributeValue );
}

bool
AqMD3::LogicDeviceWriteRegisterInt32( ViConstString logicDevice, ViInt64 offset, ViInt32 value )
{
    return clog( AqMD3_LogicDeviceWriteRegisterInt32( session_, logicDevice, offset, value ), __FILE__, __LINE__ );
}

bool
AqMD3::LogicDeviceReadRegisterInt32( ViConstString logicDevice,	ViInt64 offset,	ViInt32& value ) const
{
    return clog( AqMD3_LogicDeviceReadRegisterInt32( session_, logicDevice, offset, &value ), __FILE__, __LINE__ );
}


bool
AqMD3::LogicDeviceReadIndirectInt32( ViConstString logicDevice
                                     , ViInt32 id
                                     , ViInt64 startAddress
                                     , ViInt64 numElements
                                     , ViInt64 dataBufferSize
                                     , ViInt32 * data
                                     , ViInt64& actualElements
                                     , ViInt64& firstValidElement ) const
{
    return clog( AqMD3_LogicDeviceReadIndirectInt32( session_
                                                     , logicDevice
                                                     , id
                                                     , startAddress
                                                     , numElements
                                                     , dataBufferSize
                                                     , data
                                                     , &actualElements
                                                     , &firstValidElement )
                 , __FILE__, __LINE__ );
}

uint64_t
AqMD3::pkdTimestamp()
{
    return (*impl_->tsReader_)( *this );
}

uint32_t
AqMD3::pkdActualAverages() const
{
    ViInt32 actualAverages(0);
    LogicDeviceReadRegisterInt32( "DpuA", 0x3358, actualAverages ); // actualAverage
    return actualAverages;
}


bool
AqMD3::setSampleRate( double sampleRate )
{
    return clog( AqMD3_SetAttributeViReal64( session_, "", AQMD3_ATTR_SAMPLE_RATE, sampleRate ), __FILE__, __LINE__
                , [=](){ return (boost::format("setSampleRate( %g )") % sampleRate).str();} );
}

bool
AqMD3::QueryMinWaveformMemory( ViInt32 dataWidth, ViInt64 numRecords, ViInt64 offsetWithinRecord, ViInt64 numPointsPerRecord, ViInt64& numSamples ) const
{
    return clog( AqMD3_QueryMinWaveformMemory( session_, dataWidth, numRecords, offsetWithinRecord, numPointsPerRecord, &numSamples ), __FILE__, __LINE__ );
}

double
AqMD3::SampleRate() const
{
    double sampleRate(0);
    if ( clog( AqMD3_GetAttributeViReal64( session_, "", AQMD3_ATTR_SAMPLE_RATE, &sampleRate ), __FILE__, __LINE__ ) )
        return sampleRate;
    return 0;
}

bool
AqMD3::setActiveTriggerSource( const std::string& trigSource )
{
    return clog( AqMD3_SetAttributeViString( session_, "", AQMD3_ATTR_ACTIVE_TRIGGER_SOURCE, trigSource.c_str() ), __FILE__, __LINE__ );
}

bool
AqMD3::setTriggerDelay( double delay )
{
    return clog( AqMD3_SetAttributeViReal64( session_, "", AQMD3_ATTR_TRIGGER_DELAY, delay ), __FILE__, __LINE__
                , [=](){ return (boost::format("setTriggerDelay( %g )") % delay).str();} );
}

bool
AqMD3::setTriggerCoupling( const std::string& trigSource, int32_t coupling )
{
    return clog( AqMD3_SetAttributeViInt32( session_, trigSource.c_str(), AQMD3_ATTR_TRIGGER_COUPLING, coupling ), __FILE__, __LINE__ );
}

int32_t
AqMD3::TriggerCoupling( const std::string& trigSource ) const
{
    ViInt32 coupling(0);
    if ( clog( AqMD3_GetAttributeViInt32( session_, trigSource.c_str(), AQMD3_ATTR_TRIGGER_COUPLING, &coupling ), __FILE__, __LINE__ ) )
        return coupling;
    return (-1);
}

bool
AqMD3::setDataInversionEnabled( const std::string& channel, bool enable )
{
    ViBoolean value = enable ? VI_TRUE : VI_FALSE;
    return clog( AqMD3_SetAttributeViBoolean( session_, channel.c_str(), AQMD3_ATTR_CHANNEL_DATA_INVERSION_ENABLED, value ), __FILE__, __LINE__ );
}

bool
AqMD3::setAcquisitionRecordSize( uint32_t nbrSamples )
{
    // waveform length
    return clog( AqMD3_SetAttributeViInt64( session_, "", AQMD3_ATTR_RECORD_SIZE, nbrSamples ), __FILE__, __LINE__ );
}

bool
AqMD3::setAcquisitionNumberOfAverages( uint32_t numAverages )
{
    return clog( AqMD3_SetAttributeViInt32( session_, "", AQMD3_ATTR_ACQUISITION_NUMBER_OF_AVERAGES, numAverages ), __FILE__, __LINE__ );
}

bool
AqMD3::setAcquisitionNumRecordsToAcquire( uint32_t numRecords ) // MultiRecord
{
    // number of waveforms
    return clog( AqMD3_SetAttributeViInt64( session_, "", AQMD3_ATTR_NUM_RECORDS_TO_ACQUIRE, numRecords ), __FILE__, __LINE__ );
}

bool
AqMD3::setAcquisitionMode( int mode )
{
    return clog( AqMD3_SetAttributeViInt32( session_, "", AQMD3_ATTR_ACQUISITION_MODE, mode /* AQMD3_VAL_ACQUISITION_MODE_AVERAGER */ ), __FILE__, __LINE__ );

}

int
AqMD3::AcquisitionMode() const
{
    ViInt32 mode( 0 );
    if ( clog( AqMD3_GetAttributeViInt32( session_, "", AQMD3_ATTR_ACQUISITION_MODE, &mode ), __FILE__, __LINE__ ) )
        return mode;
    return (-1);
}

bool
AqMD3::SelfCalibrate()
{
    return clog( AqMD3_SelfCalibrate( session_ ), __FILE__, __LINE__ );
}

bool
AqMD3::AcquisitionInitiate()
{
    return clog( AqMD3_InitiateAcquisition( session_ ), __FILE__, __LINE__, []{ return std::string("AcquisitionInitiate"); } );
}

bool
AqMD3::AcquisitionWaitForAcquisitionComplete( uint32_t milliseconds )
{
    return clog( AqMD3_WaitForAcquisitionComplete( session_, milliseconds ), __FILE__, __LINE__
                , [=]{ return ( boost::format("AcquisitionWaitForComplete(%1%)") % milliseconds ).str() ; } );
}

bool
AqMD3::isAcquisitionIdle() const
{
    ViInt32 idle( AQMD3_VAL_ACQUISITION_STATUS_RESULT_FALSE );

    clog( AqMD3_GetAttributeViInt32( session_, "", AQMD3_ATTR_IS_IDLE, &idle ), __FILE__, __LINE__, [=]{ return ( boost::format("isIdle") ).str(); } );

    return idle == AQMD3_VAL_ACQUISITION_STATUS_RESULT_TRUE;
}

boost::tribool
AqMD3::isTSRAcquisitionComplete() const
{
    ViBoolean value( VI_FALSE );
    if ( clog( AqMD3_GetAttributeViBoolean( session_, "", AQMD3_ATTR_TSR_IS_ACQUISITION_COMPLETE, &value )
              , __FILE__, __LINE__, [](){ return "isTSRAcquisitionComplete()"; } ) ) {
        return value != VI_FALSE;
    }
    return boost::indeterminate;
}

boost::tribool
AqMD3::TSRMemoryOverflowOccured() const
{
    ViBoolean value( VI_FALSE );
    ViStatus rcode;

    if ( clog( ( rcode = AqMD3_GetAttributeViBoolean( session_, "", AQMD3_ATTR_TSR_MEMORY_OVERFLOW_OCCURRED, &value ) )
              , __FILE__, __LINE__, [](){ return "TSRMemoryOverflowOccured()"; } ) )
        return value == VI_FALSE ? false : true;

    return boost::indeterminate;
}

bool
AqMD3::TSRContinue()
{
    return clog( AqMD3_TSRContinue( session_ ), __FILE__, __LINE__ ) ;
}

bool
AqMD3::abort()
{
    return clog( AqMD3_Abort( session_ ), __FILE__, __LINE__, [](){ return "Abort"; } ) ;
}

bool
AqMD3::setTriggerHoldOff( double seconds )
{
    return clog( AqMD3_SetAttributeViReal64( session_, "", AQMD3_ATTR_TRIGGER_HOLDOFF, seconds ), __FILE__, __LINE__ ) ;
}

double
AqMD3::TriggerHoldOff() const
{
    double seconds(0);
    if ( clog( AqMD3_GetAttributeViReal64( session_, "", AQMD3_ATTR_TRIGGER_HOLDOFF, &seconds ), __FILE__, __LINE__ ) )
        return seconds;
    return -9999;
}

boost::tribool
AqMD3::isIdle() const
{
    ViInt32 status( 0 );
    if ( clog( AqMD3_IsIdle ( session_, &status ), __FILE__, __LINE__, [](){ return "isIdle"; }) ) {
        if ( status == AQMD3_VAL_ACQUISITION_STATUS_RESULT_TRUE )
            return true;
        else if ( status == AQMD3_VAL_ACQUISITION_STATUS_RESULT_FALSE )
            return false;
        else if ( status == AQMD3_VAL_ACQUISITION_STATUS_RESULT_UNKNOWN )
            return boost::indeterminate;
    }
    return boost::indeterminate;
}

boost::tribool
AqMD3::isMeasuring() const
{
    ViInt32 status( 0 );

    if ( clog( AqMD3_IsMeasuring( session_, &status ), __FILE__, __LINE__, [](){ return "isMeasuring"; }) ) {
        if ( status == AQMD3_VAL_ACQUISITION_STATUS_RESULT_TRUE )
            return true;
        else if ( status == AQMD3_VAL_ACQUISITION_STATUS_RESULT_FALSE )
            return false;
        else if ( status == AQMD3_VAL_ACQUISITION_STATUS_RESULT_UNKNOWN )
            return boost::indeterminate;
    }
    return boost::indeterminate;
}

#if 0 // not supported by aqmd3
boost::tribool
AqMD3::isWaitingForArm () const
{
    ViInt32 status( 0 );

    if ( clog( AqMD3_IsWaitingForArm( session_, &status ), __FILE__, __LINE__, [](){ return "isWaitingForArm"; }) ) {
        if ( status == AQMD3_VAL_ACQUISITION_STATUS_RESULT_TRUE )
            return true;
        else if ( status == AQMD3_VAL_ACQUISITION_STATUS_RESULT_FALSE )
            return false;
        else if ( status == AQMD3_VAL_ACQUISITION_STATUS_RESULT_UNKNOWN )
            return boost::indeterminate;
    }
    return boost::indeterminate;
}
#endif

boost::tribool
AqMD3::isWaitingForTrigger() const
{
    ViInt32 status( 0 );

    if ( clog( AqMD3_IsWaitingForTrigger( session_, &status ), __FILE__, __LINE__, [](){ return "isWaitingForTrigger"; }) ) {
        if ( status == AQMD3_VAL_ACQUISITION_STATUS_RESULT_TRUE )
            return true;
        else if ( status == AQMD3_VAL_ACQUISITION_STATUS_RESULT_FALSE )
            return false;
        else if ( status == AQMD3_VAL_ACQUISITION_STATUS_RESULT_UNKNOWN )
            return boost::indeterminate;
    }
    return boost::indeterminate;
}

/////////////////////////////////////////////
namespace aqmd3 {

    //////////////////////////////////////////////////////////////////////////
    // ViReal64
    template<> AQMD3SHARED_EXPORT
    ViStatus
    AqMD3::setAttribute( ViConstString _1, ViAttr _2, ViReal64 value )
    {
        return AqMD3_SetAttributeViReal64( session_, _1, _2, value );
    }

    template<> AQMD3SHARED_EXPORT
    ViStatus
    AqMD3::getAttribute( ViConstString _1, ViAttr _2, ViReal64& result ) const
    {
        return AqMD3_GetAttributeViReal64( session_, _1, _2, &result );
    }

    //////////////////////////////////////////////////////////////////////////
    // ViInt64
    template<> AQMD3SHARED_EXPORT
    ViStatus
    AqMD3::setAttribute( ViConstString _1, ViAttr _2, ViInt64 value )
    {
        return AqMD3_SetAttributeViInt64( session_, _1, _2, value );
    }

    template<> AQMD3SHARED_EXPORT
    ViStatus
    AqMD3::getAttribute( ViConstString _1, ViAttr _2, ViInt64& value ) const
    {
        return AqMD3_GetAttributeViInt64( session_, _1, _2, &value );
    }

    //////////////////////////////////////////////////////////////////////////
    // ViInt32
    template<> AQMD3SHARED_EXPORT
    ViStatus
    AqMD3::setAttribute( ViConstString _1, ViAttr _2, ViInt32 value )
    {
        return AqMD3_SetAttributeViInt32( session_, _1, _2, value );
    }

    template<> AQMD3SHARED_EXPORT
    ViStatus
    AqMD3::getAttribute( ViConstString _1, ViAttr _2, ViInt32& result ) const
    {
        return AqMD3_GetAttributeViInt32( session_, _1, _2, &result );
    }

    //////////////////////////////////////////////////////////////////////////
    // bool
    template<> AQMD3SHARED_EXPORT
    ViStatus
    AqMD3::setAttribute( ViConstString _1, ViAttr _2, ViBoolean value )
    {
        return AqMD3_SetAttributeViBoolean( session_, _1, _2, value );
    }

    template<> AQMD3SHARED_EXPORT
    ViStatus
    AqMD3::getAttribute( ViConstString _1, ViAttr _2, ViBoolean& value ) const
    {
        return AqMD3_GetAttributeViBoolean( session_, _1, _2, &value );
    }

    template<> AQMD3SHARED_EXPORT
    ViStatus
    AqMD3::setAttribute( ViConstString _1, ViAttr _2, bool value )
    {
        return AqMD3_SetAttributeViBoolean( session_, _1, _2, value ? VI_TRUE : VI_FALSE );
    }

    template<> AQMD3SHARED_EXPORT
    ViStatus
    AqMD3::getAttribute( ViConstString _1, ViAttr _2, bool& value ) const
    {
        ViBoolean tmp;
        auto rcode = AqMD3_GetAttributeViBoolean( session_, _1, _2, &tmp );
        value = tmp ? true : false;
        return rcode;
    }

    //////////////////////////////////////////////////////////////////////////
    // std::string
    template<> AQMD3SHARED_EXPORT
    ViStatus
    AqMD3::setAttribute( ViConstString _1, ViAttr _2, const std::string& value )
    {
        return AqMD3_SetAttributeViString( session_, _1, _2, value.c_str() );
    }

    template<> AQMD3SHARED_EXPORT
    ViStatus
    AqMD3::setAttribute( ViConstString _1, ViAttr _2, std::string value )
    {
        return AqMD3_SetAttributeViString( session_, _1, _2, value.c_str() );
    }


    template<> AQMD3SHARED_EXPORT
    ViStatus
    AqMD3::getAttribute( ViConstString _1, ViAttr _2, std::string& result ) const
    {
        ViChar str[128];
        result.clear();
        auto rcode = AqMD3_GetAttributeViString( session_, _1, _2, sizeof( str ), str );
        if ( rcode == VI_SUCCESS )
            result = str;
        return rcode;
    }
}
