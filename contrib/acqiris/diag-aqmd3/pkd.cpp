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
#include <adportable/debug.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>

// Edit resource and options as needed. Resource is ignored if option has Simulate=true.
// An input signal is necessary if the example is run in non simulated mode, otherwise
// the acquisition will time out.
ViChar resource[] = "PXI59::0::0::INSTR";
ViChar options[]  = "Simulate=false, DriverSetup= Model=SA230";

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
//ViInt64 const recordSize = 20000;   // 10 us acquisition at 2 GS/s
ViInt64 const recordSize = 20000 * 5;   // 10 us acquisition at 2 GS/s
ViInt64 const numRecords = 1;		// only single record supported
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

#define checkApiCall( f ) do { ViStatus s = f; testApiCall( s, #f ); } while( false )


// Utility function to check status error during driver API call.
void testApiCall( ViStatus status, char const * functionName )
{
    ViInt32 ErrorCode;
    ViChar ErrorMessage[256];

    if( status>0 ) // Warning occurred.
    {
        AqMD3_GetError( VI_NULL, &ErrorCode, sizeof(ErrorMessage), ErrorMessage );
        std::cerr << "** Warning during " << functionName << ": 0x" << std::hex << ErrorCode << ", " << ErrorMessage << "\n";

    }
    else if( status<0 ) // Error occurred.
    {
        AqMD3_GetError( VI_NULL, &ErrorCode, sizeof(ErrorMessage), ErrorMessage );
        std::cerr << "** ERROR during " << functionName << ": 0x" << std::hex << ErrorCode << ", " << ErrorMessage << "\n";
        throw std::runtime_error( ErrorMessage );
    }
}

// Compute the PKD Rising and Falling delta parameter
	//bit 15:0 --> Rising Delta in ADC codes
	//bit 31:16 --> Falling Delta in ADC codes
int32_t RisingFallingDelta(uint16_t rising, uint16_t falling)
 {
	 uint32_t const low = uint32_t(rising);
	 uint32_t const high = uint32_t(falling) << 16;
	 return high | low;
 }


int
pkd_main( std::shared_ptr< aqmd3::AqMD3 > md3, const aqmd3controls::method& m, size_t replicates )
{
    ADDEBUG() << "\nStarting Averager + PKD\n";

    // Initialize the driver. See driver help topic "Initializing the IVI-C Driver" for additional information.
    ViSession session = md3->session();
    ViBoolean idQuery = VI_FALSE;
    ViBoolean reset   = VI_FALSE;
    // checkApiCall( AqMD3_InitWithOptions( resource, idQuery, reset, options, &session ) );
    // std::cout << "\nDriver initialized\n";

    // Abort execution if instrument is still in simulated mode.
    ViBoolean simulate;

    checkApiCall( AqMD3_GetAttributeViBoolean( session, "", AQMD3_ATTR_SIMULATE, &simulate ) );
    if( simulate==VI_TRUE )
    {
        std::cout << "\nThe Averager features are not supported in simulated mode.\n";
        std::cout << "Please update the resource string (resource[]) to match your configuration,";
        std::cout << " and update the init options string (options[]) to disable simulation.\n";

        AqMD3_close( session );

        return 1;
    }
    std::cout << "\nSimulate:           " << ( simulate==VI_TRUE ? "true" : "false" ) << "\n";

    // Check the instrument contains the required AVG module option.
    ViChar str[128] = { '\0' };
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_INSTRUMENT_INFO_OPTIONS, sizeof(str), str ) );
    if( std::string( str ).find( "AVG" )== std::string::npos )
    {
        std::cout << "The required AVG module option is missing from the instrument.\n";

        AqMD3_close( session );

        return 1;
    }

    // Read and output a few attributes.
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_SPECIFIC_DRIVER_PREFIX, sizeof(str), str ) );
    std::cout << "Driver prefix:      " << str << "\n";
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_SPECIFIC_DRIVER_REVISION, sizeof(str), str ) );
    std::cout << "Driver revision:    " << str << "\n";
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_SPECIFIC_DRIVER_VENDOR, sizeof(str), str ) );
    std::cout << "Driver vendor:      " << str << "\n";
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_SPECIFIC_DRIVER_DESCRIPTION, sizeof(str), str ) );
    std::cout << "Driver description: " << str << "\n";
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_INSTRUMENT_MODEL, sizeof(str), str ) );
    std::cout << "Instrument model:   " << str << "\n";
    std::string const instrModel = str;
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_INSTRUMENT_INFO_OPTIONS, sizeof(str), str) );
    std::cout << "Instrument options: " << str << '\n';
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_INSTRUMENT_FIRMWARE_REVISION, sizeof(str), str ) );
    std::cout << "Firmware revision:  " << str << "\n";
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_INSTRUMENT_INFO_SERIAL_NUMBER_STRING, sizeof(str), str ) );
    std::cout << "Serial number:      " << str << "\n";


    // Configure the acquisition.
    std::cout << "\nConfiguring Acquisition\n";
    std::cout << "  Range:              " << range << "\n";
    std::cout << "  Offset:             " << offset << "\n";
    std::cout << "  Coupling:           " << ( coupling ? "DC" : "AC" ) << "\n";
    checkApiCall( AqMD3_ConfigureChannel( session, "Channel1", range, offset, coupling, VI_TRUE ) );
    std::cout << "  Sample rate:        " << sampleRate << "\n";
	checkApiCall(AqMD3_SetAttributeViReal64(session, "", AQMD3_ATTR_SAMPLE_RATE, sampleRate));
    std::cout << "  Record size:        " << recordSize << "\n";
    checkApiCall( AqMD3_SetAttributeViInt64( session, "", AQMD3_ATTR_RECORD_SIZE, recordSize ) );
    std::cout << "  Number of averages: " << numAverages << "\n";
    checkApiCall( AqMD3_SetAttributeViInt32( session, "", AQMD3_ATTR_ACQUISITION_NUMBER_OF_AVERAGES, numAverages ) );
    checkApiCall( AqMD3_SetAttributeViInt32( session, "", AQMD3_ATTR_ACQUISITION_MODE, AQMD3_VAL_ACQUISITION_MODE_AVERAGER ) );
    std::cout << "  Data Inversion:     " << dataInversion << "\n";
	checkApiCall( AqMD3_SetAttributeViBoolean(session, "Channel1", AQMD3_ATTR_CHANNEL_DATA_INVERSION_ENABLED, dataInversion));

    // Configure the trigger
    std::cout << "Configuring Trigger\n";
    std::cout << "  ActiveSource:       " << triggerSource << '\n';
    std::cout << "  Level:              " << triggerLevel << "\n";
	std::cout << "  Slope:              " << (triggerSlope ? "Positive" : "Negative") << "\n";
	std::cout << "  Delay:              " << triggerDelay << "\n";
	checkApiCall(AqMD3_SetAttributeViString(session, "", AQMD3_ATTR_ACTIVE_TRIGGER_SOURCE, triggerSource));
	checkApiCall(AqMD3_SetAttributeViReal64(session, triggerSource, AQMD3_ATTR_TRIGGER_LEVEL, triggerLevel));
	checkApiCall(AqMD3_SetAttributeViInt32(session, triggerSource, AQMD3_ATTR_TRIGGER_SLOPE, triggerSlope));
	checkApiCall(AqMD3_SetAttributeViReal64(session, "", AQMD3_ATTR_TRIGGER_DELAY, triggerDelay));

	// Configure Baseline Stabilisation.
	std::cout << "Configuring Baseline Stabilisation\n";
	std::cout << "  Mode:               " << blMode << '\n';
	std::cout << "  Digital Offset:     " << blDigitalOffset << '\n';
	std::cout << "  Pulse Threshold:    " << blPulseThreshold << '\n';
	std::cout << "  Pulse Polarity:     " << blPulsePolarity << '\n';
	checkApiCall(AqMD3_SetAttributeViInt32(session, "Channel1", AQMD3_ATTR_CHANNEL_BASELINE_CORRECTION_MODE, blMode));
	checkApiCall(AqMD3_SetAttributeViInt32(session, "Channel1", AQMD3_ATTR_CHANNEL_BASELINE_CORRECTION_DIGITAL_OFFSET, blDigitalOffset));
	checkApiCall(AqMD3_SetAttributeViInt32(session, "Channel1", AQMD3_ATTR_CHANNEL_BASELINE_CORRECTION_PULSE_THRESHOLD, blPulseThreshold));
	checkApiCall(AqMD3_SetAttributeViInt32(session, "Channel1", AQMD3_ATTR_CHANNEL_BASELINE_CORRECTION_PULSE_POLARITY, blPulsePolarity));

    // Calibrate the instrument.
    std::cout << "\nPerforming self-calibration\n";
    checkApiCall( AqMD3_SelfCalibrate( session ) );

	// Configure the PeakDetect
	// It is important to Configure the PeakDetect parameters after the self - calibration in your application.
	std::cout << "Configuring PeakDetect\n";
	std::cout << "  RisingDelta:      " << RisingDelta << '\n';
	std::cout << "  FallingDelta:     " << FallingDelta << '\n';
	std::cout << "  PKD parameters:  "  << "0x" << std::hex << RisingFallingDelta(RisingDelta, FallingDelta) << "\n";
	std::cout << "  AmplitudeAccumulationEnabled: " << AmplitudeAccumulationEnabled << '\n';

	// Configure PKD AmplitudeAccumulationEnabled
	if (AmplitudeAccumulationEnabled==0)
		checkApiCall(AqMD3_LogicDeviceWriteRegisterInt32(session, "DpuA", 0x33B4, 0x143511)); // PKD - Amplitude mode
	else checkApiCall(AqMD3_LogicDeviceWriteRegisterInt32(session, "DpuA", 0x33B4, 0x143515)); // PKD - Count mode

	// Configure PKD Rising and Falling Delta
	//bit 15:0 --> Rising Delta ADC codes
	//bit 31:16 --> Falling Delta ADC codes
	checkApiCall(AqMD3_LogicDeviceWriteRegisterInt32(session, "DpuA", 0x33B8, RisingFallingDelta(RisingDelta,FallingDelta))); // PKD Rising and Falling delta

	// Required to complete the PKD configuration
	checkApiCall(AqMD3_LogicDeviceWriteRegisterInt32(session, "DpuA", 0x3350, 0x00000027)); //PKD configuration

	// Readout parameters
	ViInt64 arraySize = 0;
	checkApiCall(AqMD3_QueryMinWaveformMemory(session, 32, 1, 0, recordSize, &arraySize));
    std::vector<ViInt32> dataArray(arraySize);
	ViInt64 actualPoints = 0, firstValidPoint = 0;

	// Perform the acquisition.
	std::cout << "\nPerforming acquisition\n";
	ViInt32 timeoutInMs = 1000;
	std::ofstream outputFilech1("pkd_data.txt");
	std::ofstream outputFilech2("avg_data.txt");

        checkApiCall( AqMD3_InitiateAcquisition( session ) );
        checkApiCall( AqMD3_WaitForAcquisitionComplete( session, timeoutInMs ) );
        std::cout << "Acquisition completed\n";

	std::cout << "Read the Peak histogram\n";
	ViInt64 addressLow = 0x00000000;
	ViInt32 addressHigh_Ch1 = 0x00000080; // To read the Peak Histogram on CH1
	checkApiCall(AqMD3_LogicDeviceReadIndirectInt32(session, "DpuA", addressHigh_Ch1, addressLow, recordSize, arraySize, &dataArray[0], &actualPoints, &firstValidPoint));

	for (ViInt64 currentPoint = 0; currentPoint < actualPoints; ++currentPoint)	{
		ViInt32 const valueRaw = dataArray[firstValidPoint + currentPoint];
		outputFilech1 << valueRaw << "\n";
	}

	std::cout << "Read the accumulated RAW data\n";
	ViInt32 addressHigh_Ch2 = 0x00000090; // To read the accumulated raw data on CH2
	checkApiCall(AqMD3_LogicDeviceReadIndirectInt32(session, "DpuA", addressHigh_Ch2, addressLow, recordSize, arraySize, &dataArray[0], &actualPoints, &firstValidPoint));


	for (ViInt64 currentPoint = 0; currentPoint < actualPoints; ++currentPoint)	{
		ViInt32 const valueRaw = dataArray[firstValidPoint + currentPoint];
		outputFilech2 << valueRaw << "\n";
	}

    std::cout << "\nProcessing completed\n";

	outputFilech1.close();
	outputFilech2.close();

    // Close the driver.
    checkApiCall( AqMD3_close( session ) );
    std::cout << "Driver closed \n";

    return 0;
}
