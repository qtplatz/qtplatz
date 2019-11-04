///
/// Acqiris IVI-C Driver Example Program
///
/// Initializes the driver, reads a few Identity interface properties, and performs a
/// simple record acquisition.
///
/// For additional information on programming with IVI drivers in various IDEs, please see
/// http://www.ivifoundation.org/resources/
///
/// Runs in simulation mode without an instrument.
///

#include "AqMD3.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::hex;
#include <vector>
using std::vector;
#include <stdexcept>
using std::runtime_error;

#define checkApiCall( f ) do { ViStatus s = f; testApiCall( s, #f ); } while( false )

// Edit resource and options as needed. Resource is ignored if option has Simulate=true.
// An input signal is necessary if the example is run in non simulated mode, otherwise
// the acquisition will time out.
ViChar resource[] = "PXI40::0::0::INSTR";
ViChar options[]  = "Simulate=true, DriverSetup= Model=U5303A";


ViInt64 const recordSize = 1000000;
ViInt64 const numRecords = 1;

// Utility function to check status error during driver API call.
void testApiCall( ViStatus status, char const * functionName )
{
    ViInt32 ErrorCode;
    ViChar ErrorMessage[512];

    if( status>0 ) // Warning occurred.
    {
        AqMD3_GetError( VI_NULL, &ErrorCode, sizeof( ErrorMessage ), ErrorMessage );
        cerr << "** Warning during " << functionName << ": 0x" << hex << ErrorCode << ", " << ErrorMessage << '\n';

    }
    else if( status<0 ) // Error occurred.
    {
        AqMD3_GetError( VI_NULL, &ErrorCode, sizeof( ErrorMessage ), ErrorMessage );
        cerr << "** ERROR during " << functionName << ": 0x" << hex << ErrorCode << ", " << ErrorMessage << '\n';
        throw runtime_error( ErrorMessage );
    }
}


int main()
{
    cout << "SimpleAcquisition\n\n";

    // Initialize the driver. See driver help topic "Initializing the IVI-C Driver" for additional information.
    ViSession session;
    ViBoolean const idQuery = VI_FALSE;
    ViBoolean const reset   = VI_FALSE;
    checkApiCall( AqMD3_InitWithOptions( resource, idQuery, reset, options, &session ) );

    cout << "Driver initialized \n";

    // Read and output a few attributes.
    ViChar str[128];
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_SPECIFIC_DRIVER_PREFIX,               sizeof( str ), str ) );
    cout << "Driver prefix:      " << str << '\n';
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_SPECIFIC_DRIVER_REVISION,             sizeof( str ), str ) );
    cout << "Driver revision:    " << str << '\n';
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_SPECIFIC_DRIVER_VENDOR,               sizeof( str ), str ) );
    cout << "Driver vendor:      " << str << '\n';
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_SPECIFIC_DRIVER_DESCRIPTION,          sizeof( str ), str ) );
    cout << "Driver description: " << str << '\n';
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_INSTRUMENT_MODEL,                     sizeof( str ), str ) );
    cout << "Instrument model:   " << str << '\n';
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_INSTRUMENT_INFO_OPTIONS,              sizeof( str ), str ) );
    cout << "Instrument options: " << str << '\n';
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_INSTRUMENT_FIRMWARE_REVISION,         sizeof( str ), str ) );
    cout << "Firmware revision:  " << str << '\n';
    checkApiCall( AqMD3_GetAttributeViString( session, "", AQMD3_ATTR_INSTRUMENT_INFO_SERIAL_NUMBER_STRING, sizeof( str ), str ) );
    cout << "Serial number:      " << str << '\n';

    ViBoolean simulate;
    checkApiCall( AqMD3_GetAttributeViBoolean( session, "", AQMD3_ATTR_SIMULATE, &simulate ) );
    cout << "\nSimulate:           " << ( simulate?"True":"False" ) << '\n';

    // Configure the acquisition.
    ViReal64 const range = 1.0;
    ViReal64 const offset = 0.0;
    ViInt32 const coupling = AQMD3_VAL_VERTICAL_COUPLING_DC;
    cout << "\nConfiguring acquisition\n";
    cout << "Range:              " << range << '\n';
    cout << "Offset:             " << offset << '\n';
    cout << "Coupling:           " << ( coupling?"DC":"AC" ) << '\n';
    checkApiCall( AqMD3_ConfigureChannel( session, "Channel1", range, offset, coupling, VI_TRUE ) );
    cout << "Number of records:  " << numRecords << '\n';
    cout << "Record size:        " << recordSize << '\n';
    checkApiCall( AqMD3_SetAttributeViInt64( session, "", AQMD3_ATTR_NUM_RECORDS_TO_ACQUIRE, numRecords ) );
    checkApiCall( AqMD3_SetAttributeViInt64( session, "", AQMD3_ATTR_RECORD_SIZE,            recordSize ) );

    // Configure the trigger.
    cout << "\nConfiguring trigger\n";
    checkApiCall( AqMD3_SetAttributeViString( session, "", AQMD3_ATTR_ACTIVE_TRIGGER_SOURCE, "Internal1" ) );

    // Calibrate the instrument.
    cout << "\nPerforming self-calibration\n";
    checkApiCall( AqMD3_SelfCalibrate( session ) );

    // Perform the acquisition.
    ViInt32 const timeoutInMs = 1000;
    cout << "\nPerforming acquisition\n";
    checkApiCall( AqMD3_InitiateAcquisition( session ) );
    checkApiCall( AqMD3_WaitForAcquisitionComplete( session, timeoutInMs ) );
    cout << "Acquisition completed\n";

    // Fetch the acquired data in array.
    ViInt64 arraySize = 0;
    checkApiCall( AqMD3_QueryMinWaveformMemory( session, 16, numRecords, 0, recordSize, &arraySize ) );

    vector<ViInt16> dataArray( arraySize );
    ViInt64 actualPoints, firstValidPoint;
    ViReal64 initialXOffset[numRecords], initialXTimeSeconds[numRecords], initialXTimeFraction[numRecords];
    ViReal64 xIncrement = 0.0, scaleFactor = 0.0, scaleOffset = 0.0;
    checkApiCall( AqMD3_FetchWaveformInt16( session, "Channel1", arraySize, &dataArray[0],
         &actualPoints, &firstValidPoint, initialXOffset, initialXTimeSeconds, initialXTimeFraction,
         &xIncrement, &scaleFactor, &scaleOffset ) );

    // Convert data to Volts.
    cout << "\nProcessing data\n";
    for( ViInt64 currentPoint = 0; currentPoint< actualPoints; ++currentPoint )
    {
        ViReal64 valueInVolts = dataArray[firstValidPoint + currentPoint]*scaleFactor + scaleOffset;
    }

    cout << "Processing completed\n";

    // Close the driver.
    checkApiCall( AqMD3_close( session ) );
    cout << "\nDriver closed\n";

    return 0;
}

