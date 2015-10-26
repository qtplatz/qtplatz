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

#include "agmd2.hpp"
#include <acqrscontrols/u5303a/identify.hpp>
#include <acqrscontrols/u5303a/waveform.hpp>
#include <adportable/debug.hpp>
#include <adlog/logger.hpp>
#include <boost/format.hpp>
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
    
}

using namespace u5303a;

AgMD2::AgMD2() : session_( 0 )
               , impl_( new impl() )
{
}

AgMD2::~AgMD2()
{
    AgMD2_close( session_ );    
}

bool
AgMD2::InitWithOptions( const std::string& resource, ViBoolean idQuery, ViBoolean reset, const std::string& options )
{
    return log( AgMD2_InitWithOptions( const_cast< char *>(resource.c_str())
                                       , idQuery, reset, options.c_str(), &session_ ), __FILE__, __LINE__ );
}
        
bool
AgMD2::log( ViStatus rcode, const char * const file, int line )
{
    if ( rcode ) {
        ViInt32 errorCode;
        ViChar msg[256];
        AgMD2_GetError( VI_NULL, &errorCode, sizeof(msg), msg );
        adportable::debug(file, line) << boost::format("0x%x::0x%x: %s") % rcode % errorCode % msg;
        adlog::logger(file,line,(rcode < 0 ? adlog::LOG_ERROR : adlog::LOG_WARN)) << boost::format("0x%x: %s") % errorCode % msg;
    }
    return rcode == VI_SUCCESS;
}

bool
AgMD2::GetAttributeViString ( ViStatus& rcode, ViConstString RepCapIdentifier, ViAttr AttributeID, std::string& result )
{
    ViChar str[128];

    result.clear();
    if ( (rcode = AgMD2_GetAttributeViString ( session_, RepCapIdentifier, AttributeID, sizeof(str), str )) == VI_SUCCESS )
        result = str;
    log( rcode, __FILE__, __LINE__ );

    return rcode == VI_SUCCESS;
}

bool
AgMD2::Identify( std::shared_ptr< acqrscontrols::u5303a::identify >& ident )
{
    if ( !ident )
        ident = std::make_shared< acqrscontrols::u5303a::identify >();

    ViStatus rcode(0);
    std::string str;

    if ( GetAttributeViString( rcode, "", AGMD2_ATTR_SPECIFIC_DRIVER_PREFIX, str ) )
        ident->Identifier() = str;

    if ( GetAttributeViString( rcode, "", AGMD2_ATTR_SPECIFIC_DRIVER_REVISION, str ) )
        ident->Revision() = str;

    if ( GetAttributeViString( rcode, "", AGMD2_ATTR_SPECIFIC_DRIVER_VENDOR, str ) )
        ident->Vendor() = str;
            
    if ( GetAttributeViString( rcode, "", AGMD2_ATTR_SPECIFIC_DRIVER_DESCRIPTION, str ) )
        ident->Description() = str;
            
    if ( GetAttributeViString( rcode, "", AGMD2_ATTR_INSTRUMENT_MODEL, str) )
        ident->InstrumentModel() = str;

    if ( GetAttributeViString( rcode, "", AGMD2_ATTR_INSTRUMENT_INFO_OPTIONS, str) )
        ident->Options() = str;
        
    if ( GetAttributeViString( rcode, "", AGMD2_ATTR_INSTRUMENT_FIRMWARE_REVISION, str ) )
        ident->FirmwareRevision() = str;
                 
    if ( GetAttributeViString( rcode, "", AGMD2_ATTR_INSTRUMENT_INFO_SERIAL_NUMBER_STRING, str ) )
        ident->SerialNumber() = str;
            
    if ( GetAttributeViString( rcode, "", AGMD2_ATTR_INSTRUMENT_INFO_IO_VERSION, str ) )
        ident->IOVersion() = str;

    int32_t nbrADCBits(0);
    if ( AgMD2_GetAttributeViInt32( rcode, "", AGMD2_ATTR_INSTRUMENT_INFO_NBR_ADC_BITS, &nbrADCBits ) )
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

bool
AgMD2::isSimulate() const
{
    ViBoolean simulate(false);
    return log( AgMD2_GetAttributeViBoolean( session_, "", AGMD2_ATTR_SIMULATE, &simulate ), __FILE__, __LINE__ );
}

bool
AgMD2::setSampleRate( double sampleRate )
{
    return log( AgMD2_SetAttributeViReal64( session_, "", AGMD2_ATTR_SAMPLE_RATE, sampleRate ), __FILE__, __LINE__ );
}

double
AgMD2::SampleRate() const
{
    double sampleRate(0);
    if ( log( AgMD2_GetAttributeViReal64( session_, "", AGMD2_ATTR_SAMPLE_RATE, &sampleRate ), __FILE__, __LINE__ ) )
        return sampleRate;
    return 0;
}

bool
AgMD2::setActiveTriggerSource( const std::string& trigSource )
{
    return log( AgMD2_SetAttributeViString( session_, "", AGMD2_ATTR_ACTIVE_TRIGGER_SOURCE, trigSource.c_str() ), __FILE__, __LINE__ );
}

bool
AgMD2::setTriggerDelay( double delay )
{
    return log( AgMD2_SetAttributeViReal64( session_, "", AGMD2_ATTR_TRIGGER_DELAY, delay ), __FILE__, __LINE__ );
}

bool
AgMD2::setTriggerCoupling( int32_t coupling )
{
    return log( AgMD2_SetAttributeViInt32( session_, "", AGMD2_ATTR_TRIGGER_COUPLING, coupling ), __FILE__, __LINE__ );
}

bool
AgMD2::setTriggerLevel( const std::string& trigSource,  double level )
{
    return log( AgMD2_SetAttributeViReal64( session_, trigSource.c_str(), AGMD2_ATTR_TRIGGER_LEVEL, level ), __FILE__, __LINE__ );
}

double
AgMD2::TriggerLevel( const std::string& trigSource ) const
{
    double level(0);

    if ( log( AgMD2_GetAttributeViReal64( session_, trigSource.c_str(), AGMD2_ATTR_TRIGGER_LEVEL, &level ), __FILE__, __LINE__ ) )
        return level;

    return -9999;
}


bool
AgMD2::setTriggerSlope( const std::string& trigSource, int32_t slope )
{
    return log( AgMD2_SetAttributeViInt32( session_, trigSource.c_str(), AGMD2_ATTR_TRIGGER_SLOPE, slope ), __FILE__, __LINE__ );
}

int32_t
AgMD2::TriggerSlope( const std::string& trigSource ) const
{
    int32_t slope(0);  // NEGATIVE = 0, POSITIVE = 1

    if ( log( AgMD2_GetAttributeViInt32( session_, trigSource.c_str(), AGMD2_ATTR_TRIGGER_SLOPE, &slope ), __FILE__, __LINE__ ) )
        return slope;

    return 0;
}


bool
AgMD2::setDataInversionEnabled( bool enable )
{
    ViBoolean value = enable ? VI_TRUE : VI_FALSE;
    return log( AgMD2_SetAttributeViBoolean( session_, "", AGMD2_ATTR_CHANNEL_DATA_INVERSION_ENABLED, value ), __FILE__, __LINE__ );
}

bool
AgMD2::setAcquisitionRecordSize( uint32_t nbrSamples )
{
    // waveform length
    return log( AgMD2_SetAttributeViInt64( session_, "", AGMD2_ATTR_RECORD_SIZE, nbrSamples ), __FILE__, __LINE__ );
}

bool
AgMD2::setAcquisitionNumberOfAverages( uint32_t numAverages )
{
    return log( AgMD2_SetAttributeViInt32( session_, "", AGMD2_ATTR_ACQUISITION_NUMBER_OF_AVERAGES, numAverages ), __FILE__, __LINE__ );
}

bool
AgMD2::setAcquisitionNumRecordsToAcquire( uint32_t numRecords ) // MultiRecord
{
    // number of waveforms
    return log( AgMD2_SetAttributeViInt64( session_, "", AGMD2_ATTR_NUM_RECORDS_TO_ACQUIRE, numRecords ), __FILE__, __LINE__ );
}

bool
AgMD2::setAcquisitionMode( int mode )
{
    return log( AgMD2_SetAttributeViInt32( session_, "", AGMD2_ATTR_ACQUISITION_MODE, mode /* AGMD2_VAL_ACQUISITION_MODE_AVERAGER */ ), __FILE__, __LINE__ );

}

bool
AgMD2::CalibrationSelfCalibrate()
{
    return log( AgMD2_SelfCalibrate( session_ ), __FILE__, __LINE__ );    
}

bool
AgMD2::AcquisitionInitiate()
{
    return log( AgMD2_InitiateAcquisition( session_ ), __FILE__, __LINE__ );
}

bool
AgMD2::AcquisitionWaitForAcquisitionComplete( uint32_t milliseconds )
{
    return log( AgMD2_WaitForAcquisitionComplete( session_, milliseconds ), __FILE__, __LINE__ );
}

