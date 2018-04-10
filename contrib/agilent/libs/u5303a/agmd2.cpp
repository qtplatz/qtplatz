/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "agmd2.hpp"
#include <acqrscontrols/u5303a/identify.hpp>
#include <acqrscontrols/u5303a/waveform.hpp>
#include <adportable/debug.hpp>
#include <adlog/logger.hpp>
#include <boost/format.hpp>
#include <boost/logic/tribool.hpp>
#include <atomic>

namespace u5303a {

    class AgMD2::impl {

        std::atomic< uint32_t > dataSerialNumber_;
    public:
        std::shared_ptr< acqrscontrols::u5303a::identify > ident_;
        
    public:
        impl() : dataSerialNumber_( 0 ) {
        }

        uint32_t dataSerialNumber() {
            return dataSerialNumber_++;
        }
    };

    enum AgMD2ATTRIBUTES {
    };
    
}

namespace u5303a {

    //////////////////////////////////////////////////////////////////////////
    // ViReal64
    template<>
    ViStatus
    AgMD2::setAttribute( ViConstString _1, ViAttr _2, ViReal64 value )
    {
        return AgMD2_SetAttributeViReal64( session_, _1, _2, value );
    }

    template<>
    ViStatus
    AgMD2::getAttribute( ViConstString _1, ViAttr _2, ViReal64& result ) const
    {
        return AgMD2_GetAttributeViReal64( session_, _1, _2, &result );
    }

    //////////////////////////////////////////////////////////////////////////
    // ViInt64
    template<>
    ViStatus
    AgMD2::setAttribute( ViConstString _1, ViAttr _2, ViInt64 value )
    {
        return AgMD2_SetAttributeViInt64( session_, _1, _2, value );
    }

    template<>
    ViStatus
    AgMD2::getAttribute( ViConstString _1, ViAttr _2, ViInt64& value ) const
    {
        return AgMD2_GetAttributeViInt64( session_, _1, _2, &value );
    }

    //////////////////////////////////////////////////////////////////////////
    // ViInt32
    template<>
    ViStatus
    AgMD2::setAttribute( ViConstString _1, ViAttr _2, ViInt32 value )
    {
        return AgMD2_SetAttributeViInt32( session_, _1, _2, value );
    }

    template<>
    ViStatus
    AgMD2::getAttribute( ViConstString _1, ViAttr _2, ViInt32& result ) const
    {
        return AgMD2_GetAttributeViInt32( session_, _1, _2, &result );
    }

    //////////////////////////////////////////////////////////////////////////
    // bool
    template<>
    ViStatus
    AgMD2::setAttribute( ViConstString _1, ViAttr _2, ViBoolean value )
    {
        return AgMD2_SetAttributeViBoolean( session_, _1, _2, value );
    }

    template<>
    ViStatus
    AgMD2::getAttribute( ViConstString _1, ViAttr _2, ViBoolean& value ) const
    {
        return AgMD2_GetAttributeViBoolean( session_, _1, _2, &value );
    }

    template<>
    ViStatus
    AgMD2::setAttribute( ViConstString _1, ViAttr _2, bool value )
    {
        return AgMD2_SetAttributeViBoolean( session_, _1, _2, value ? VI_TRUE : VI_FALSE );
    }

    template<>
    ViStatus
    AgMD2::getAttribute( ViConstString _1, ViAttr _2, bool& value ) const
    {
        ViBoolean tmp;
        auto rcode = AgMD2_GetAttributeViBoolean( session_, _1, _2, &tmp );
        value = tmp ? true : false;
        return rcode;
    }

    //////////////////////////////////////////////////////////////////////////
    // std::string
    template<>
    ViStatus
    AgMD2::setAttribute( ViConstString _1, ViAttr _2, const std::string& value )
    {
        return AgMD2_SetAttributeViString( session_, _1, _2, value.c_str() );
    }

    template<>
    ViStatus
    AgMD2::setAttribute( ViConstString _1, ViAttr _2, std::string value )
    {
        return AgMD2_SetAttributeViString( session_, _1, _2, value.c_str() );
    }
    

    template<>
    ViStatus
    AgMD2::getAttribute( ViConstString _1, ViAttr _2, std::string& result ) const
    {
        ViChar str[128];
        result.clear();
        auto rcode = AgMD2_GetAttributeViString( session_, _1, _2, sizeof( str ), str );
        if ( rcode == VI_SUCCESS )
            result = str;
        return rcode;
    }

    // template<> template<> ViStatus attribute< AGMD2_ATTR_ACTIVE_TRIGGER_SOURCE >::set( AgMD2& _, const std::string& value ) {
    //     return _.setAttribute< AGMD2_ATTR_ACTIVE_TRIGGER_SOURCE >( "", value );
    // }
    // template<> template<> ViStatus attribute< AGMD2_ATTR_ACTIVE_TRIGGER_SOURCE >::get( AgMD2& _, std::string& value ) const {
    //     return _.getAttribute< AGMD2_ATTR_ACTIVE_TRIGGER_SOURCE >( "", value );
    // }
}

using namespace u5303a;

AgMD2::AgMD2() : session_( 0 )
               , impl_( new impl() )
{
}

AgMD2::~AgMD2()
{
    //AgMD2_close( session_ );
}

bool
AgMD2::log( ViStatus rcode, const char * const file, int line, std::function< std::string()> describe )
{
    if ( rcode ) {
        ViInt32 errorCode;
        ViChar msg[256];
        AgMD2_GetError( VI_NULL, &errorCode, sizeof(msg), msg );
        if ( describe ) {

            adlog::logger(file,line,(rcode < 0 ? adlog::LOG_ERR : adlog::LOG_WARNING)) << boost::format("0x%x: %s where %s")
                % errorCode % msg % describe();

        } else {

            adlog::logger(file,line,(rcode < 0 ? adlog::LOG_ERR : adlog::LOG_WARNING)) << boost::format("0x%x: %s ") % errorCode % msg;            
        }
    }
    return rcode == VI_SUCCESS;
}

ViStatus
AgMD2::initWithOptions( const std::string& resource, ViBoolean idQuery, ViBoolean reset, const std::string& options )
{
    return AgMD2_InitWithOptions( const_cast< char *>(resource.c_str())
                                  , idQuery
                                  , reset
                                  , options.c_str()
                                  , &session_ );
}
        
ViStatus
AgMD2::getAttributeViString ( ViConstString RepCapIdentifier, ViAttr AttributeID, std::string& result ) const
{
    return getAttribute<>( RepCapIdentifier, AttributeID, result );
}

ViStatus
AgMD2::getAttributeViInt32 ( ViConstString RepCapIdentifier, ViAttr AttributeID, int32_t& result ) const
{
    return getAttribute<>( RepCapIdentifier, AttributeID, result );
}

bool
AgMD2::Identify( std::shared_ptr< acqrscontrols::u5303a::identify >& ident )
{
    if ( !ident )
        ident = std::make_shared< acqrscontrols::u5303a::identify >();

    ViStatus rcode(0);
    std::string str;

    if ( log( getAttributeViString( "", AGMD2_ATTR_SPECIFIC_DRIVER_PREFIX, str ), __FILE__,__LINE__ ) )
        ident->Identifier() = str;

    if ( log( getAttributeViString( "", AGMD2_ATTR_SPECIFIC_DRIVER_REVISION, str ), __FILE__,__LINE__ ) )
        ident->Revision() = str;

    if ( log( getAttributeViString( "", AGMD2_ATTR_SPECIFIC_DRIVER_VENDOR, str ), __FILE__,__LINE__ ) )
        ident->Vendor() = str;
            
    if ( log( getAttributeViString( "", AGMD2_ATTR_SPECIFIC_DRIVER_DESCRIPTION, str ), __FILE__,__LINE__ ) )
        ident->Description() = str;
            
    if ( log( getAttributeViString( "", AGMD2_ATTR_INSTRUMENT_MODEL, str), __FILE__,__LINE__ ) )
        ident->InstrumentModel() = str;

    if ( log( getAttributeViString( "", AGMD2_ATTR_INSTRUMENT_INFO_OPTIONS, str ), __FILE__,__LINE__ ) )
        ident->Options() = str;
        
    if ( log( getAttributeViString( "", AGMD2_ATTR_INSTRUMENT_FIRMWARE_REVISION, str ), __FILE__,__LINE__ ) )
        ident->FirmwareRevision() = str;
                 
    if ( log( getAttributeViString( "", AGMD2_ATTR_INSTRUMENT_INFO_SERIAL_NUMBER_STRING, str ), __FILE__,__LINE__ ) )
        ident->SerialNumber() = str;
            
    if ( log( getAttributeViString( "", AGMD2_ATTR_INSTRUMENT_INFO_IO_VERSION, str ), __FILE__,__LINE__ ) )
        ident->IOVersion() = str;

    int32_t nbrADCBits(0);
    if ( log( getAttribute( "", AGMD2_ATTR_INSTRUMENT_INFO_NBR_ADC_BITS, nbrADCBits ), __FILE__,__LINE__ ) )
        // if ( log( getAttribute< AGMD2_ATTR_INSTRUMENT_INFO_NBR_ADC_BITS >( "", nbrADCBits ), __FILE__,__LINE__) )
        ident->NbrADCBits() = nbrADCBits;

    impl_->ident_ = ident;

    return true;
}

std::shared_ptr< acqrscontrols::u5303a::identify >
AgMD2::Identify()
{
    return impl_->ident_;
}

uint32_t
AgMD2::dataSerialNumber()
{
    return impl_->dataSerialNumber();
}

bool
AgMD2::ConfigureTimeInterleavedChannelList( const std::string& channelName, const std::string& channelList )
{
    return log( AgMD2_ConfigureTimeInterleavedChannelList( session_, channelName.c_str(), channelList.c_str() ), __FILE__, __LINE__ );
}

// bool
// AgMD2::isSimulate() const
// {
//     ViBoolean simulate(false);
//     return log( AgMD2_GetAttributeViBoolean( session_, "", AGMD2_ATTR_SIMULATE, &simulate ), __FILE__, __LINE__ );
// }

// Added for PKD+AVG POC quick test
ViStatus
AgMD2::setAttributeViInt32( ViConstString RepCapIdentifier, ViAttr AttributeID, ViInt32 AttributeValue )
{
    return AgMD2_SetAttributeViInt32( session_, RepCapIdentifier, AttributeID, AttributeValue );
}

ViStatus
AgMD2::setAttributeViInt64( ViConstString RepCapIdentifier, ViAttr AttributeID, ViInt64 AttributeValue )
{
    return AgMD2_SetAttributeViInt32( session_, RepCapIdentifier, AttributeID, AttributeValue );
}

ViStatus
AgMD2::setAttributeViBoolean( ViConstString RepCapIdentifier, ViAttr AttributeID, ViBoolean AttributeValue )
{
    return AgMD2_SetAttributeViBoolean( session_, RepCapIdentifier, AttributeID, AttributeValue );
}

// ViStatus
// AgMD2::setSampleRate( double sampleRate )
// {
//     return AgMD2_SetAttributeViReal64( session_, "", AGMD2_ATTR_SAMPLE_RATE, sampleRate );
// }

// double
// AgMD2::SampleRate() const
// {
//     double sampleRate(0);
//     if ( log( AgMD2_GetAttributeViReal64( session_, "", AGMD2_ATTR_SAMPLE_RATE, &sampleRate ), __FILE__, __LINE__ ) )
//         return sampleRate;
//     return 0;
// }

// bool
// AgMD2::setActiveTriggerSource( const std::string& trigSource )
// {
//     return log( AgMD2_SetAttributeViString( session_, "", AGMD2_ATTR_ACTIVE_TRIGGER_SOURCE, trigSource.c_str() ), __FILE__, __LINE__ );
// }

// bool
// AgMD2::setTriggerDelay( double delay )
// {
//     return log( AgMD2_SetAttributeViReal64( session_, "", AGMD2_ATTR_TRIGGER_DELAY, delay ), __FILE__, __LINE__
//                 , [=](){ return (boost::format("setTriggerDelay( %g )") % delay).str();} );
// }

// bool
// AgMD2::setTriggerCoupling( const std::string& trigSource, int32_t coupling )
// {
//     return log( AgMD2_SetAttributeViInt32( session_, trigSource.c_str(), AGMD2_ATTR_TRIGGER_COUPLING, coupling ), __FILE__, __LINE__ );
// }

// int32_t
// AgMD2::TriggerCoupling( const std::string& trigSource ) const
// {
//     ViInt32 coupling(0);
//     if ( log( AgMD2_GetAttributeViInt32( session_, trigSource.c_str(), AGMD2_ATTR_TRIGGER_COUPLING, &coupling ), __FILE__, __LINE__ ) )
//         return coupling;
//     return (-1);
// }

// bool
// AgMD2::setTriggerLevel( const std::string& trigSource,  double level )
// {
//     return log( AgMD2_SetAttributeViReal64( session_, trigSource.c_str(), AGMD2_ATTR_TRIGGER_LEVEL, level ), __FILE__, __LINE__ );
// }

// double
// AgMD2::TriggerLevel( const std::string& trigSource ) const
// {
//     double level(0);

//     if ( log( AgMD2_GetAttributeViReal64( session_, trigSource.c_str(), AGMD2_ATTR_TRIGGER_LEVEL, &level ), __FILE__, __LINE__ ) )
//         return level;

//     return -9999;
// }


// bool
// AgMD2::setTriggerSlope( const std::string& trigSource, int32_t slope )
// {
//     return log( AgMD2_SetAttributeViInt32( session_, trigSource.c_str(), AGMD2_ATTR_TRIGGER_SLOPE, slope ), __FILE__, __LINE__ );
// }

// int32_t
// AgMD2::TriggerSlope( const std::string& trigSource ) const
// {
//     int32_t slope(0);  // NEGATIVE = 0, POSITIVE = 1

//     if ( log( AgMD2_GetAttributeViInt32( session_, trigSource.c_str(), AGMD2_ATTR_TRIGGER_SLOPE, reinterpret_cast<ViInt32*>(&slope) ), __FILE__, __LINE__ ) )
//         return slope;

//     return 0;
// }


// bool
// AgMD2::setDataInversionEnabled( const std::string& channel, bool enable )
// {
//     ViBoolean value = enable ? VI_TRUE : VI_FALSE;
//     return log( AgMD2_SetAttributeViBoolean( session_, channel.c_str(), AGMD2_ATTR_CHANNEL_DATA_INVERSION_ENABLED, value ), __FILE__, __LINE__ );
// }

// bool
// AgMD2::setAcquisitionRecordSize( uint32_t nbrSamples )
// {
//     // waveform length
//     return log( AgMD2_SetAttributeViInt64( session_, "", AGMD2_ATTR_RECORD_SIZE, nbrSamples ), __FILE__, __LINE__ );
// }

// bool
// AgMD2::setAcquisitionNumberOfAverages( uint32_t numAverages )
// {
//     return log( AgMD2_SetAttributeViInt32( session_, "", AGMD2_ATTR_ACQUISITION_NUMBER_OF_AVERAGES, numAverages ), __FILE__, __LINE__ );
// }

// bool
// AgMD2::setAcquisitionNumRecordsToAcquire( uint32_t numRecords ) // MultiRecord
// {
//     // number of waveforms
//     return log( AgMD2_SetAttributeViInt64( session_, "", AGMD2_ATTR_NUM_RECORDS_TO_ACQUIRE, numRecords ), __FILE__, __LINE__ );
// }

// bool
// AgMD2::setAcquisitionMode( int mode )
// {
//     return log( AgMD2_SetAttributeViInt32( session_, "", AGMD2_ATTR_ACQUISITION_MODE, mode /* AGMD2_VAL_ACQUISITION_MODE_AVERAGER */ ), __FILE__, __LINE__ );

// }

// int
// AgMD2::AcquisitionMode() const
// {
//     ViInt32 mode( 0 );
//     if ( log( AgMD2_GetAttributeViInt32( session_, "", AGMD2_ATTR_ACQUISITION_MODE, &mode ), __FILE__, __LINE__ ) )
//         return mode;
//     return (-1);
// }

bool
AgMD2::CalibrationSelfCalibrate()
{
    return log( AgMD2_SelfCalibrate( session_ ), __FILE__, __LINE__ );    
}

bool
AgMD2::AcquisitionInitiate()
{
    return log( AgMD2_InitiateAcquisition( session_ ), __FILE__, __LINE__, []{ return std::string("AcquisitionInitiate"); } );
}

bool
AgMD2::AcquisitionWaitForAcquisitionComplete( uint32_t milliseconds )
{
    return log( AgMD2_WaitForAcquisitionComplete( session_, milliseconds ), __FILE__, __LINE__
                , [=]{ return ( boost::format("AcquisitionWaitForComplete(%1%)") % milliseconds ).str() ; } );
}

bool
AgMD2::isAcquisitionIdle() const
{
    ViInt32 idle( AGMD2_VAL_ACQUISITION_STATUS_RESULT_FALSE );
    
    log( AgMD2_GetAttributeViInt32( session_, "", AGMD2_ATTR_IS_IDLE, &idle ), __FILE__, __LINE__, [=]{ return ( boost::format("isIdle") ).str(); } );

    return idle == AGMD2_VAL_ACQUISITION_STATUS_RESULT_TRUE;
}

// bool
// AgMD2::setTSREnabled( bool enable )
// {
//     ViBoolean value = enable ? VI_TRUE : VI_FALSE;
//     return log( AgMD2_SetAttributeViBoolean( session_, "", AGMD2_ATTR_TSR_ENABLED, value ), __FILE__, __LINE__ );
// }

// bool
// AgMD2::TSREnabled()
// {
//     ViBoolean value( VI_FALSE );
//     log( AgMD2_GetAttributeViBoolean( session_, "", AGMD2_ATTR_TSR_ENABLED, &value ), __FILE__, __LINE__ );
//     return value == VI_FALSE ? false : true;
// }

boost::tribool
AgMD2::isTSRAcquisitionComplete() const
{
    ViBoolean value( VI_FALSE );
    if ( log( AgMD2_GetAttributeViBoolean( session_, "", AGMD2_ATTR_TSR_IS_ACQUISITION_COMPLETE, &value ) 
              , __FILE__, __LINE__, [](){ return "isTSRAcquisitionComplete()"; } ) ) {
        return value != VI_FALSE;
    }
    return boost::indeterminate;
}

boost::tribool
AgMD2::TSRMemoryOverflowOccured() const
{
    ViBoolean value( VI_FALSE );
    ViStatus rcode;

    if ( log( ( rcode = AgMD2_GetAttributeViBoolean( session_, "", AGMD2_ATTR_TSR_MEMORY_OVERFLOW_OCCURRED, &value ) )
              , __FILE__, __LINE__, [](){ return "TSRMemoryOverflowOccured()"; } ) )
        return value == VI_FALSE ? false : true;
    
    return boost::indeterminate;
}

bool
AgMD2::TSRContinue()
{
    ViBoolean value( VI_FALSE );
    return log( AgMD2_TSRContinue( session_ ), __FILE__, __LINE__ ) ;
}

bool
AgMD2::abort()
{
    ViBoolean value( VI_FALSE );
    return log( AgMD2_Abort( session_ ), __FILE__, __LINE__, [](){ return "Abort"; } ) ;
}

bool
AgMD2::setTriggerHoldOff( double seconds )
{
    return log( AgMD2_SetAttributeViReal64( session_, "", AGMD2_ATTR_TRIGGER_HOLDOFF, seconds ), __FILE__, __LINE__ ) ;
}

double
AgMD2::TriggerHoldOff() const
{
    double seconds(0);
    if ( log( AgMD2_GetAttributeViReal64( session_, "", AGMD2_ATTR_TRIGGER_HOLDOFF, &seconds ), __FILE__, __LINE__ ) )
        return seconds;
    return -9999;
}

boost::tribool
AgMD2::isIdle() const
{
    ViInt32 status( 0 );
    if ( log( AgMD2_IsIdle ( session_, &status ), __FILE__, __LINE__, [](){ return "isIdle"; }) ) {
        if ( status == AGMD2_VAL_ACQUISITION_STATUS_RESULT_TRUE )
            return true;
        else if ( status == AGMD2_VAL_ACQUISITION_STATUS_RESULT_FALSE )
            return false;
        else if ( status == AGMD2_VAL_ACQUISITION_STATUS_RESULT_UNKNOWN )
            return boost::indeterminate;
    }
    return boost::indeterminate;
}

boost::tribool
AgMD2::isMeasuring() const
{
    ViInt32 status( 0 );

    if ( log( AgMD2_IsMeasuring( session_, &status ), __FILE__, __LINE__, [](){ return "isMeasuring"; }) ) {
        if ( status == AGMD2_VAL_ACQUISITION_STATUS_RESULT_TRUE )
            return true;
        else if ( status == AGMD2_VAL_ACQUISITION_STATUS_RESULT_FALSE )
            return false;
        else if ( status == AGMD2_VAL_ACQUISITION_STATUS_RESULT_UNKNOWN )
            return boost::indeterminate;
    }
    return boost::indeterminate;
}

boost::tribool
AgMD2::isWaitingForArm () const
{
    ViInt32 status( 0 );
    
    if ( log( AgMD2_IsWaitingForArm( session_, &status ), __FILE__, __LINE__, [](){ return "isWaitingForArm"; }) ) {
        if ( status == AGMD2_VAL_ACQUISITION_STATUS_RESULT_TRUE )
            return true;
        else if ( status == AGMD2_VAL_ACQUISITION_STATUS_RESULT_FALSE )
            return false;
        else if ( status == AGMD2_VAL_ACQUISITION_STATUS_RESULT_UNKNOWN )
            return boost::indeterminate;
    }
    return boost::indeterminate;
}

boost::tribool
AgMD2::isWaitingForTrigger() const
{
    ViInt32 status( 0 );
    
    if ( log( AgMD2_IsWaitingForTrigger( session_, &status ), __FILE__, __LINE__, [](){ return "isWaitingForTrigger"; }) ) {
        if ( status == AGMD2_VAL_ACQUISITION_STATUS_RESULT_TRUE )
            return true;
        else if ( status == AGMD2_VAL_ACQUISITION_STATUS_RESULT_FALSE )
            return false;
        else if ( status == AGMD2_VAL_ACQUISITION_STATUS_RESULT_UNKNOWN )
            return boost::indeterminate;
    }
    return boost::indeterminate;    
}
