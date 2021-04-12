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
//ViReal64 const range = 0.5;
//ViReal64 const offset = 0.0;
ViInt32 const coupling = AQMD3_VAL_VERTICAL_COUPLING_DC;
ViBoolean dataInversion = false;

// Baseline Stabilisation parameters -  see baseline stabilization for more information on the configuration and settings
ViInt32 const blMode = AQMD3_VAL_BASELINE_CORRECTION_MODE_CONTINUOUS; // set blMode to AQMD3_VAL_BASELINE_CORRECTION_MODE_DISABLED to disable it
ViInt32 const blDigitalOffset = 0;
ViInt32 const blPulseThreshold = 500;
// ViInt32 const blPulsePolarity = AQMD3_VAL_BASELINE_CORRECTION_PULSE_POLARITY_POSITIVE;

// Acquisition parameters
ViReal64 const sampleRate = 2e9;	// only 2 GS/s supported
ViInt64 const recordSize = 20000 * 5;   // 10 us acquisition at 2 GS/s
ViInt32 const numAverages = 104;

// PDK parameters
//ViUInt16 const RisingDelta = 500;  // defines in ADC count the amount by which two consecutive samples must differ to be considered as rising edge in the peak detection algorithm
                                       // it can be setup from 0 to 16383
//ViUInt16 const FallingDelta = 500; // defines in ADC count the amount by which two consecutive samples must differ to be considered as falling edge in the peak detection algorithm
                                       // it can be setup from 0 to 16383
//ViInt32 const AmplitudeAccumulationEnabled = 1; // selects if the peak value is stored (0) or the peak value is forced to (1).

// Trigger parameters
ViConstString triggerSource = "External1";
ViReal64 const triggerLevel = 0.0;
ViInt32 const triggerSlope = AQMD3_VAL_TRIGGER_SLOPE_POSITIVE;
ViReal64 const triggerDelay = 300e-6; // trigger delay in s

// Compute the PKD Rising and Falling delta parameter
	//bit 15:0 --> Rising Delta in ADC codes
	//bit 31:16 --> Falling Delta in ADC codes
// int32_t RisingFallingDelta(uint16_t rising, uint16_t falling)
//  {
// 	 uint32_t const low = uint32_t(rising);
// 	 uint32_t const high = uint32_t(falling) << 16;
// 	 return high | low;
//  }

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
    ADDEBUG() << "\nStarting Averager + PKD";

    // Initialize the driver. See driver help topic "Initializing the IVI-C Driver" for additional information.
    ViSession session = md3->session();
    // Abort execution if instrument is still in simulated mode.
    using aqmd3::AqMD3;
    using aqmd3::attribute;

    ViStatus rcode;
    if ( auto simulate = aqmd3::attribute< aqmd3::simulate >::value( *md3, rcode ) ) {
        if ( *simulate == VI_TRUE ) {
            std::cout << "\nThe Averager features are not supported in simulated mode.";
            std::cout << "Please update the resource string (resource[]) to match your configuration,";
            std::cout << " and update the init options string (options[]) to disable simulation.";
        }
    } else {
        md3->clog( rcode, __FILE__, __LINE__ );
        return 1;
    }

    if ( md3->Identify()->Options().find( "AVG" ) == std::string::npos ) {
        ADDEBUG() << "The required AVG module option is missing from the instrument.";
        return 1;
    }

    // Configure the acquisition.
    ADDEBUG() << "\nConfiguring Acquisition";
    ADDEBUG() << "  Range:              " << m.device_method().front_end_range;
    ADDEBUG() << "  Offset:             " << m.device_method().front_end_offset;
    ADDEBUG() << "  Coupling:           " << ( coupling ? "DC" : "AC" ) << "";

    md3->ConfigureChannel( "Channel1", m.device_method().front_end_range, m.device_method().front_end_offset, AQMD3_VAL_VERTICAL_COUPLING_DC, VI_TRUE );
    ADDEBUG() << "  Sample rate:        " << sampleRate << "";
    aqmd3::attribute< aqmd3::sample_rate >::set ( *md3, sampleRate );
	// md3->clog(AqMD3_SetAttributeViReal64(session, "", AQMD3_ATTR_SAMPLE_RATE, sampleRate));
    ADDEBUG() << "  Record size:        " << recordSize << ", " << m.device_method().nbr_of_s_to_acquire_;
    // md3->clog( AqMD3_SetAttributeViInt64( session, "", AQMD3_ATTR_RECORD_SIZE, recordSize ) );
    aqmd3::attribute< aqmd3::record_size >::set( *md3, m.device_method().nbr_of_s_to_acquire_ );
    ADDEBUG() << "  Number of averages: " << numAverages << "";
    // md3->clog( AqMD3_SetAttributeViInt32( session, "", AQMD3_ATTR_ACQUISITION_NUMBER_OF_AVERAGES, numAverages ) );
    aqmd3::attribute< aqmd3::acquisition_number_of_averages >::set( *md3, m.device_method().nbr_of_averages );

    // md3->clog( AqMD3_SetAttributeViInt32( session, "", AQMD3_ATTR_ACQUISITION_MODE, AQMD3_VAL_ACQUISITION_MODE_AVERAGER ) );
    aqmd3::attribute< aqmd3::acquisition_mode >::set( *md3, AQMD3_VAL_ACQUISITION_MODE_AVERAGER );
    ADDEBUG() << "  Data Inversion:     " << dataInversion << "";
	md3->clog( AqMD3_SetAttributeViBoolean(session, "Channel1", AQMD3_ATTR_CHANNEL_DATA_INVERSION_ENABLED, dataInversion));
    aqmd3::attribute< aqmd3::channel_data_inversion_enabled >::set( *md3, m.device_method().invert_signal );

    // Configure the trigger
    ADDEBUG() << "Configuring Trigger";
    ADDEBUG() << "  ActiveSource:       " << triggerSource << '\n';
    ADDEBUG() << "  Level:              " << triggerLevel << "";
	ADDEBUG() << "  Slope:              " << (triggerSlope ? "Positive" : "Negative") << "";
	ADDEBUG() << "  Delay:              " << triggerDelay << "";

    md3->clog( aqmd3::attribute< aqmd3::active_trigger_source >::set( *md3, triggerSource ), __FILE__, __LINE__ );
    md3->clog( aqmd3::attribute< aqmd3::trigger_level >::set( *md3, triggerSource, m.device_method().ext_trigger_level ), __FILE__, __LINE__ );
    md3->clog( aqmd3::attribute< aqmd3::trigger_slope >::set( *md3, triggerSource, triggerSlope ), __FILE__, __LINE__ );
    md3->clog( aqmd3::attribute< aqmd3::trigger_delay >::set( *md3, m.device_method().delay_to_first_sample_ ),  __FILE__, __LINE__ );

	// Configure Baseline Stabilisation.
    auto blPulsePolarity = m.device_method().invert_signal ? aqmd3::BASELINE_CORRECTION_PULSE_POLARITY_POSITIVE : aqmd3::BASELINE_CORRECTION_PULSE_POLARITY_NEGATIVE;
	ADDEBUG() << "Configuring Baseline Stabilisation";
	ADDEBUG() << "  Mode:               " << blMode;
	ADDEBUG() << "  Digital Offset:     " << blDigitalOffset; // 0
	ADDEBUG() << "  Pulse Threshold:    " << blPulseThreshold; // 500
	ADDEBUG() << "  Pulse Polarity:     " << blPulsePolarity;  // POSITIVE

	md3->clog( aqmd3::attribute< aqmd3::channel_baseline_correction_mode >::set( *md3, "Channel1", aqmd3::BASELINE_CORRECTION_MODE_CONTINUOUS ), __FILE__, __LINE__ );
	md3->clog( aqmd3::attribute< aqmd3::channel_baseline_correction_digital_offset >::set( *md3, "Channel1", blDigitalOffset), __FILE__, __LINE__ );
	md3->clog( aqmd3::attribute< aqmd3::channel_baseline_correction_pulse_threshold >::set( *md3, "Channel1", blPulseThreshold), __FILE__, __LINE__ );
	md3->clog( aqmd3::attribute< aqmd3::channel_baseline_correction_pulse_polarity >::set( *md3, "Channel1", blPulsePolarity ), __FILE__, __LINE__ );

    // Calibrate the instrument.
    ADDEBUG() << "\nPerforming self-calibration";
    md3->SelfCalibrate();

	// Configure the PeakDetect
	// It is important to Configure the PeakDetect parameters after the self - calibration in your application.
	ADDEBUG() << "Configuring PeakDetect";
	ADDEBUG() << "  RisingDelta:      " << m.device_method().pkd_rising_delta;
	ADDEBUG() << "  FallingDelta:     " << m.device_method().pkd_falling_delta;
	ADDEBUG() << "  AmplitudeAccumulationEnabled: " << m.device_method().pkd_amplitude_accumulation_enabled;

	// Configure PKD AmplitudeAccumulationEnabled
	if ( m.device_method().pkd_amplitude_accumulation_enabled )
		md3->clog(AqMD3_LogicDeviceWriteRegisterInt32(session, "DpuA", 0x33B4, 0x143511)); // PKD - Amplitude mode
	else
        md3->clog(AqMD3_LogicDeviceWriteRegisterInt32(session, "DpuA", 0x33B4, 0x143515)); // PKD - Count mode

	// Configure PKD Rising and Falling Delta
	// bit 15:0 --> Rising Delta ADC codes
	// bit 31:16 --> Falling Delta ADC codes
    // PKD Rising and Falling delta
	md3->LogicDeviceWriteRegisterInt32( "DpuA", 0x33B8, m.device_method().pkd_rising_delta | (m.device_method().pkd_falling_delta << 16) );

	// Required to complete the PKD configuration
	md3->LogicDeviceWriteRegisterInt32( "DpuA", 0x3350, 0x00000027 ); //PKD configuration

	// Readout parameters
	ViInt64 arraySize = 0;
	if ( md3->QueryMinWaveformMemory( 32, 1, 0, recordSize, arraySize) ) {
        data d1 = {0}, d2 = {0};
        std::vector<ViInt32> pkd( arraySize ), avg( arraySize );
        ADDEBUG() << "\nPerforming acquisition";

        md3->AcquisitionInitiate();
        md3->AcquisitionWaitForAcquisitionComplete( 3000 );
        ADDEBUG() << "Acquisition completed";

        ADDEBUG() << "Read the Peak histogram";
        ViInt64 addressLow = 0x00000000;
        ViInt32 addressHigh_Ch1 = 0x00000080; // To read the Peak Histogram on CH1
        md3->LogicDeviceReadIndirectInt32( "DpuA", addressHigh_Ch1, addressLow, recordSize, arraySize, pkd.data(), d1.actualPoints, d1.firstValidPoint );

        ADDEBUG() << "Read the accumulated RAW data";
        ViInt32 addressHigh_Ch2 = 0x00000090; // To read the accumulated raw data on CH2
        md3->LogicDeviceReadIndirectInt32( "DpuA", addressHigh_Ch2, addressLow, recordSize, arraySize, avg.data(), d2.actualPoints, d2.firstValidPoint );

        for ( size_t i = 0; i < d1.actualPoints && i < d2.actualPoints; ++i)	{
            auto v1 = pkd[ d1.firstValidPoint + i ];
            auto v2 = avg[ d2.firstValidPoint + i ];
            std::cout << i << "\t" << v1 << "\t" << v2 << std::endl;
        }

        ADDEBUG() << "Processing completed";
        md3.reset();
        ADDEBUG() << "Driver closed ";
    }
    return 0;
}
