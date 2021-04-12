///
///  IVI-C Driver Example Program
///
/// Initializes the driver, reads a few Identity interface properties, and performs an
/// accumulated acquisition.
///
/// For additional information on programming with IVI drivers in various IDEs, please see
/// http://www.ivifoundation.org/resources/
///
/// WARNING:
/// The Averager features are not supported in simulation mode. You will have to update
/// the resource string (resource[]) to match your configuration and disable the
/// simulation mode (options[] - set Simulate=false) to be able to run this example
/// successfully.
///

#include "AqMD3.h"
#include <aqmd3/aqmd3.hpp>
#include <aqmd3/findresource.hpp>
#include <aqmd3controls/method.hpp>
#include <aqmd3controls/identify.hpp>
#include <adportable/debug.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>

// Edit resource and options as needed. Resource is ignored if option has Simulate=true.
// An input signal is necessary if the example is run in non simulated mode, otherwise
// the acquisition will time out.
//ViChar resource[] = "PXI59::0::0::INSTR";
//ViChar options[]  = "Simulate=false, DriverSetup= Model=SA230";

// Channel 1 parameters
ViReal64 const range = 0.5;
ViReal64 const offset = 0.0;
ViInt32 const coupling = AQMD3_VAL_VERTICAL_COUPLING_DC;
ViBoolean dataInversion = false;

// Baseline Stabilisation parameters -  see baseline stabilization for more information on the configuration and settings
ViInt32 const blMode = AQMD3_VAL_BASELINE_CORRECTION_MODE_CONTINUOUS; // set blMode to AQMD3_VAL_BASELINE_CORRECTION_MODE_DISABLED to disable it
ViInt32 const blDigitalOffset = 0;
ViInt32 const blPulseThreshold = 500;
ViInt32 const blPulsePolarity = AQMD3_VAL_BASELINE_CORRECTION_PULSE_POLARITY_POSITIVE;

// Acquisition parameters
ViReal64 const sampleRate = 2e9;	// only 2 GS/s supported
ViInt64 const recordSize = 20000 * 5;   // 10 us acquisition at 2 GS/s
ViInt32 const numAverages = 104;

// PDK parameters
ViUInt16 const RisingDelta = 500;  // defines in ADC count the amount by which two consecutive samples must differ to be considered as rising edge in the peak detection algorithm
                                       // it can be setup from 0 to 16383
ViUInt16 const FallingDelta = 500; // defines in ADC count the amount by which two consecutive samples must differ to be considered as falling edge in the peak detection algorithm
                                       // it can be setup from 0 to 16383
ViInt32 const AmplitudeAccumulationEnabled = 1; // selects if the peak value is stored (0) or the peak value is forced to (1).

// Trigger parameters
ViConstString triggerSource = "External1";
ViReal64 const triggerLevel = 0.0;
ViInt32 const triggerSlope = AQMD3_VAL_TRIGGER_SLOPE_POSITIVE;
ViReal64 const triggerDelay = 300e-6; // trigger delay in s

// Compute the PKD Rising and Falling delta parameter
	//bit 15:0 --> Rising Delta in ADC codes
	//bit 31:16 --> Falling Delta in ADC codes
int32_t RisingFallingDelta(uint16_t rising, uint16_t falling)
 {
	 uint32_t const low = uint32_t(rising);
	 uint32_t const high = uint32_t(falling) << 16;
	 return high | low;
 }

struct data {
    ViInt32 actualAverages;
    ViInt64 actualRecords;
    ViInt64 waveformArrayActualSize;
    ViInt64 actualPoints; // [numRecords];
    ViInt64 firstValidPoint; // [numRecords];
    ViReal64 initialXOffset;
    ViReal64 initialXTimeSeconds; // [numRecords];
    ViReal64 initialXTimeFraction; // [numRecords];
    ViReal64 xIncrement, scaleFactor, scaleOffset;
    ViInt32 flags; // [numRecords];
    void print( std::ostream& o, const char * heading ) const {
        std::cout << heading << ":\t"
                  << boost::format( "actualAverages: %d\tactualPoints\t%d\tfirstValidPoint\t%d" ) % actualAverages % actualPoints % firstValidPoint
                  << boost::format( "\tinitialXOffset: %d\tinitialXTime: %g" ) % initialXOffset % ( initialXTimeSeconds + initialXTimeFraction )
            // << boost::format( "\txIncrement: %d\tscaleFactor: %g\tscaleOffset: %g\tflags: 0x%x" ) % xIncrement % scaleFactor % scaleOffset % flags
                  << std::endl;
    }
};

int
pkd_main( std::shared_ptr< aqmd3::AqMD3 > md3, const aqmd3controls::method& m, size_t replicates )
{
    ADDEBUG() << "\nStarting Averager + PKD\n";

    // Initialize the driver. See driver help topic "Initializing the IVI-C Driver" for additional information.
    ViSession session = md3->session();
    //ViBoolean idQuery = VI_FALSE;
    //ViBoolean reset   = VI_FALSE;
    // checkApiCall( AqMD3_InitWithOptions( resource, idQuery, reset, options, &session ) );
    // std::cout << "\nDriver initialized\n";

    // Abort execution if instrument is still in simulated mode.
    using aqmd3::AqMD3;
    using aqmd3::attribute;

    ViStatus rcode;
    if ( auto simulate = aqmd3::attribute< aqmd3::simulate >::value( *md3, rcode ) ) {
        if ( *simulate == VI_TRUE ) {
            std::cout << "\nThe Averager features are not supported in simulated mode.\n";
            std::cout << "Please update the resource string (resource[]) to match your configuration,";
            std::cout << " and update the init options string (options[]) to disable simulation.\n";
        }
    } else {
        md3->clog( rcode, __FILE__, __LINE__ );
        return 1;
    }

    // md3->clog( AqMD3_GetAttributeViBoolean( session, "", AQMD3_ATTR_SIMULATE, &simulate ) );
    // if( simulate==VI_TRUE )
    // {
    //     AqMD3_close( session );

    //     return 1;
    // }
    // std::cout << "\nSimulate:           " << ( simulate==VI_TRUE ? "true" : "false" ) << "\n";

    // Check the instrument contains the required AVG module option.
    // ViChar str[128] = { '\0' };
    // md3->clog( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_INSTRUMENT_INFO_OPTIONS, sizeof(str), str ) );
    // if ( std::string( str ).find( "AVG" )== std::string::npos )
    // {
    //     std::cout << "The required AVG module option is missing from the instrument.\n";

    //     AqMD3_close( session );

    //     return 1;
    // }
    if ( md3->Identify()->Options().find( "AVG" ) == std::string::npos ) {
        ADDEBUG() << "The required AVG module option is missing from the instrument.\n";
        return 1;
    }

    // // Read and output a few attributes.
    // md3->clog( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_SPECIFIC_DRIVER_PREFIX, sizeof(str), str ) );
    // std::cout << "Driver prefix:      " << str << "\n";
    // md3->clog( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_SPECIFIC_DRIVER_REVISION, sizeof(str), str ) );
    // std::cout << "Driver revision:    " << str << "\n";
    // md3->clog( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_SPECIFIC_DRIVER_VENDOR, sizeof(str), str ) );
    // std::cout << "Driver vendor:      " << str << "\n";
    // md3->clog( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_SPECIFIC_DRIVER_DESCRIPTION, sizeof(str), str ) );
    // std::cout << "Driver description: " << str << "\n";
    // md3->clog( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_INSTRUMENT_MODEL, sizeof(str), str ) );
    // std::cout << "Instrument model:   " << str << "\n";
    // std::string const instrModel = str;
    // md3->clog( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_INSTRUMENT_INFO_OPTIONS, sizeof(str), str) );
    // std::cout << "Instrument options: " << str << '\n';
    // md3->clog( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_INSTRUMENT_FIRMWARE_REVISION, sizeof(str), str ) );
    // std::cout << "Firmware revision:  " << str << "\n";
    // md3->clog( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_INSTRUMENT_INFO_SERIAL_NUMBER_STRING, sizeof(str), str ) );
    // std::cout << "Serial number:      " << str << "\n";

    // Configure the acquisition.
    ADDEBUG() << "\nConfiguring Acquisition\n";
    ADDEBUG() << "  Range:              " << range << "\n";
    ADDEBUG() << "  Offset:             " << offset << "\n";
    ADDEBUG() << "  Coupling:           " << ( coupling ? "DC" : "AC" ) << "\n";

    md3->ConfigureChannel( "Channel1", range, offset, coupling, VI_TRUE );
    ADDEBUG() << "  Sample rate:        " << sampleRate << "\n";
	md3->clog(AqMD3_SetAttributeViReal64(session, "", AQMD3_ATTR_SAMPLE_RATE, sampleRate));
    ADDEBUG() << "  Record size:        " << recordSize << "\n";
    md3->clog( AqMD3_SetAttributeViInt64( session, "", AQMD3_ATTR_RECORD_SIZE, recordSize ) );
    ADDEBUG() << "  Number of averages: " << numAverages << "\n";
    md3->clog( AqMD3_SetAttributeViInt32( session, "", AQMD3_ATTR_ACQUISITION_NUMBER_OF_AVERAGES, numAverages ) );
    md3->clog( AqMD3_SetAttributeViInt32( session, "", AQMD3_ATTR_ACQUISITION_MODE, AQMD3_VAL_ACQUISITION_MODE_AVERAGER ) );
    ADDEBUG() << "  Data Inversion:     " << dataInversion << "\n";
	md3->clog( AqMD3_SetAttributeViBoolean(session, "Channel1", AQMD3_ATTR_CHANNEL_DATA_INVERSION_ENABLED, dataInversion));

    // Configure the trigger
    ADDEBUG() << "Configuring Trigger\n";
    ADDEBUG() << "  ActiveSource:       " << triggerSource << '\n';
    ADDEBUG() << "  Level:              " << triggerLevel << "\n";
	ADDEBUG() << "  Slope:              " << (triggerSlope ? "Positive" : "Negative") << "\n";
	ADDEBUG() << "  Delay:              " << triggerDelay << "\n";

    // md3->clog(AqMD3_SetAttributeViString(session, "", AQMD3_ATTR_ACTIVE_TRIGGER_SOURCE, triggerSource));
    // md3->clog(AqMD3_SetAttributeViReal64(session, triggerSource, AQMD3_ATTR_TRIGGER_LEVEL, triggerLevel));
    // md3->clog(AqMD3_SetAttributeViInt32(session, triggerSource, AQMD3_ATTR_TRIGGER_SLOPE, triggerSlope));
	// md3->clog(AqMD3_SetAttributeViReal64(session, "", AQMD3_ATTR_TRIGGER_DELAY, triggerDelay));
    md3->clog( aqmd3::attribute< aqmd3::active_trigger_source >::set( *md3, triggerSource ), __FILE__, __LINE__ );
    md3->clog( aqmd3::attribute< aqmd3::trigger_level >::set( *md3, "External1", triggerLevel ), __FILE__, __LINE__ );
    md3->clog( aqmd3::attribute< aqmd3::trigger_slope >::set( *md3, "External1", triggerSlope ), __FILE__, __LINE__ );
    md3->clog( aqmd3::attribute< aqmd3::trigger_delay >::set( *md3, triggerDelay ),  __FILE__, __LINE__ );

	// Configure Baseline Stabilisation.
	ADDEBUG() << "Configuring Baseline Stabilisation\n";
	ADDEBUG() << "  Mode:               " << blMode << '\n';
	ADDEBUG() << "  Digital Offset:     " << blDigitalOffset << '\n';
	ADDEBUG() << "  Pulse Threshold:    " << blPulseThreshold << '\n';

	ADDEBUG() << "  Pulse Polarity:     " << blPulsePolarity << '\n';

	// md3->clog(AqMD3_SetAttributeViInt32(session, "Channel1", AQMD3_ATTR_CHANNEL_BASELINE_CORRECTION_MODE, blMode));
	// md3->clog(AqMD3_SetAttributeViInt32(session, "Channel1", AQMD3_ATTR_CHANNEL_BASELINE_CORRECTION_DIGITAL_OFFSET, blDigitalOffset));
	// md3->clog(AqMD3_SetAttributeViInt32(session, "Channel1", AQMD3_ATTR_CHANNEL_BASELINE_CORRECTION_PULSE_THRESHOLD, blPulseThreshold));
	// md3->clog(AqMD3_SetAttributeViInt32(session, "Channel1", AQMD3_ATTR_CHANNEL_BASELINE_CORRECTION_PULSE_POLARITY, blPulsePolarity));
	md3->clog( aqmd3::attribute< aqmd3::channel_baseline_correction_mode >::set( *md3, "Channel1", aqmd3::BASELINE_CORRECTION_MODE_CONTINUOUS ), __FILE__, __LINE__ );
	md3->clog( aqmd3::attribute< aqmd3::channel_baseline_correction_digital_offset >::set( *md3, "Channel1", blDigitalOffset), __FILE__, __LINE__ );
	md3->clog( aqmd3::attribute< aqmd3::channel_baseline_correction_pulse_threshold >::set( *md3, "Channel1", blPulseThreshold), __FILE__, __LINE__ );
	md3->clog( aqmd3::attribute< aqmd3::channel_baseline_correction_pulse_polarity >::set( *md3, "Channel1", aqmd3::BASELINE_CORRECTION_PULSE_POLARITY_POSITIVE ), __FILE__, __LINE__ );

    // Calibrate the instrument.
    ADDEBUG() << "\nPerforming self-calibration\n";
    md3->SelfCalibrate();

	// Configure the PeakDetect
	// It is important to Configure the PeakDetect parameters after the self - calibration in your application.
	ADDEBUG() << "Configuring PeakDetect\n";
	ADDEBUG() << "  RisingDelta:      " << RisingDelta << '\n';
	ADDEBUG() << "  FallingDelta:     " << FallingDelta << '\n';
	ADDEBUG() << "  PKD parameters:  "  << "0x" << std::hex << RisingFallingDelta(RisingDelta, FallingDelta) << "\n";
	ADDEBUG() << "  AmplitudeAccumulationEnabled: " << AmplitudeAccumulationEnabled << '\n';

	// Configure PKD AmplitudeAccumulationEnabled
	if (AmplitudeAccumulationEnabled==0)
		md3->clog(AqMD3_LogicDeviceWriteRegisterInt32(session, "DpuA", 0x33B4, 0x143511)); // PKD - Amplitude mode
	else
        md3->clog(AqMD3_LogicDeviceWriteRegisterInt32(session, "DpuA", 0x33B4, 0x143515)); // PKD - Count mode

	// Configure PKD Rising and Falling Delta
	//bit 15:0 --> Rising Delta ADC codes
	//bit 31:16 --> Falling Delta ADC codes
	md3->LogicDeviceWriteRegisterInt32( "DpuA", 0x33B8, RisingFallingDelta(RisingDelta,FallingDelta) ); // PKD Rising and Falling delta

	// Required to complete the PKD configuration
	md3->LogicDeviceWriteRegisterInt32( "DpuA", 0x3350, 0x00000027 ); //PKD configuration

	// Readout parameters
	ViInt64 arraySize = 0;
	if ( md3->QueryMinWaveformMemory( 32, 1, 0, recordSize, arraySize) ) {
        data d1 = {0}, d2 = {0};
        std::vector<ViInt32> pkd( arraySize ), avg( arraySize );
        // ViInt64 actualPoints = 0, firstValidPoint = 0;
        // Perform the acquisition.
        ADDEBUG() << "\nPerforming acquisition\n";
        // ViInt32 timeoutInMs = 1000;
        // std::ofstream outputFilech1("pkd_data.txt");
        // std::ofstream outputFilech2("avg_data.txt");

        md3->AcquisitionInitiate();
        md3->AcquisitionWaitForAcquisitionComplete( 3000 );
        // md3->clog( AqMD3_InitiateAcquisition( session ) );
        // md3->clog( AqMD3_WaitForAcquisitionComplete( session, timeoutInMs ) );
        ADDEBUG() << "Acquisition completed\n";

        ADDEBUG() << "Read the Peak histogram\n";
        ViInt64 addressLow = 0x00000000;
        ViInt32 addressHigh_Ch1 = 0x00000080; // To read the Peak Histogram on CH1
        md3->LogicDeviceReadIndirectInt32( "DpuA", addressHigh_Ch1, addressLow, recordSize, arraySize, pkd.data(), d1.actualPoints, d1.firstValidPoint );

        // for (ViInt64 currentPoint = 0; currentPoint < actualPoints; ++currentPoint)	{
        //     ViInt32 const valueRaw = dataArray[firstValidPoint + currentPoint];
        //     outputFilech1 << valueRaw << "\n";
        // }

        ADDEBUG() << "Read the accumulated RAW data\n";
        ViInt32 addressHigh_Ch2 = 0x00000090; // To read the accumulated raw data on CH2
        md3->LogicDeviceReadIndirectInt32( "DpuA", addressHigh_Ch2, addressLow, recordSize, arraySize, avg.data(), d2.actualPoints, d2.firstValidPoint );

        for ( size_t i = 0; i < d1.actualPoints && i < d2.actualPoints; ++i)	{
            auto v1 = pkd[ d1.firstValidPoint + i ];
            auto v2 = avg[ d2.firstValidPoint + i ];
            std::cout << v1 << "\t" << v2 << std::endl;
        }

        ADDEBUG() << "\nProcessing completed\n";
        // outputFilech1.close();
        // outputFilech2.close();
        // Close the driver.
        md3.reset();
        ADDEBUG() << "Driver closed \n";
    }
    return 0;
}
