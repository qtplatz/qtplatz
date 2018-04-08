/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include <libdgpio/pio.hpp>
#include <u5303a/ppio.hpp>
#include <u5303a/agmd2.hpp>
#include <u5303a/digitizer.hpp>
#include <acqrscontrols/u5303a/identify.hpp>
#include <acqrscontrols/u5303a/method.hpp>
#include <boost/program_options.hpp>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <ostream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

namespace po = boost::program_options;

int __verbose__ = 5;

class execStatistics {
    execStatistics( const execStatistics& ) = delete; // non copyable

public:
    double last_;
    size_t deadCount_;
    size_t dataCount_;
    double rate_;
    std::chrono::steady_clock::time_point tp_;
    std::vector< std::pair< double, double > > exceededTimings_;

    execStatistics() : last_( 0 )
                     , deadCount_( 0 )
                     , dataCount_( 0 )
                     , rate_( 0 )
                     , tp_( std::chrono::steady_clock::now() ) {
    }

    inline double difference_from_last( double seconds ) {
        return seconds - last_;
    }

    static execStatistics& instance() {

        // lasy initialized singleton

        static execStatistics __instance;
        return __instance;

    }
};

std::ostream& operator << ( std::ostream& out, const execStatistics& t )
{
    uint64_t ns = std::chrono::duration_cast< std::chrono::nanoseconds >( std::chrono::steady_clock::now() - t.tp_ ).count();
    double s = double(ns) * 1.0e-9;
    
    out << "Took " << s << " seconds; " << double(t.dataCount_) / s << "Hz" << std::endl;
    out << t.deadCount_ << " DEAD data received / " << t.dataCount_ << " waveforms read" << std::endl;
    out << t.exceededTimings_.size() << " waveforms has the trigger timing exceeded from external trig rate of " << t.rate_ << std::endl;
    
    for ( auto& pair: t.exceededTimings_ )
        out << "waveform at " << pair.first << " interval: " << pair.second << std::endl;
    return out;
}

#if defined __linux

#include <csignal>

static void sigint(int num )
{
    std::cout << "************* got signal " << num << " ***************" << std::endl;

    std::cout << execStatistics::instance() << std::endl;
    
    std::cout << "Merci, Salut" << std::endl;
    exit( num );
}

#endif

void pkd_main( std::shared_ptr< u5303a::AgMD2 >, const acqrscontrols::u5303a::method& );
int pkd_main( const acqrscontrols::u5303a::method& );
int
main( int argc, char * argv [] )
{
    bool success( false ), simulated( false );
    bool TSR_enabled( false );

    po::variables_map vm;
    po::options_description description( "test_u5303a" );
    {
        description.add_options()
            ( "help,h",    "Display this help message" )
            ( "tsr,t",     "TSR enable" )
            ( "pkd",       "PKD enable" )
            ( "avg",       "AVG enable" )
            ( "records,r",  po::value<int>()->default_value( 1 ), "Number of records" )
            ( "average,a",  po::value<int>()->default_value( 0 ), "Number of average" )
            ( "raising-delta", po::value<int>()->default_value( 20 ), "PKD Raising delta" )
            ( "falling-delta", po::value<int>()->default_value( 20 ), "PKD Falling delta" )
            ( "mode,m",     po::value<int>()->default_value( 0 ), "=0 Normal(digitizer); =2 Averager" )
            ( "delay,d",    po::value<double>()->default_value( 0.0 ), "Delay (us)" )
            ( "width,w",    po::value<double>()->default_value( 100.0 ), "Waveform width (us)" )
            ( "replicates", po::value<int>()->default_value( 1000 ), "Number of triggers to acquire waveforms" )
            ( "rate",       po::value<double>()->default_value( 1.0 ),  "Trigger interval in millisecond" )
            ( "verbose",    po::value<int>()->default_value( 5 ),  "Verbose 0..9" )            
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }
    
    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    if ( vm.count( "tsr" ) ) {
        TSR_enabled = true;
    }

    if ( vm.count( "verbose" ) )
        __verbose__ = vm[ "verbose" ].as< int >();

    acqrscontrols::u5303a::method method;
    method.setChannels( 0x01 );
    method.setMode( static_cast<acqrscontrols::u5303a::method::DigiMode>( vm[ "mode" ].as<int>() ) );
    method._device_method().front_end_range = 1.0;  // V
    method._device_method().front_end_offset = 0.0; // V
    method._device_method().ext_trigger_level = 1.0;
    method._device_method().samp_rate = 3.2e9;

    // MultiRecords or single record
    method._device_method().nbr_records = vm[ "records" ].as<int>();
    method._device_method().nbr_of_averages = vm[ "average" ].as<int>();    // 0 for digitizer

    // delay
    method._device_method().delay_to_first_sample_
        = method._device_method().digitizer_delay_to_first_sample
        = vm[ "delay" ].as<double>() * 1.0e-6;

    // data length
    uint32_t width = uint32_t( ( vm[ "width" ].as<double>() * 1.0e-6 ) * method._device_method().samp_rate + 0.5 );
    method._device_method().digitizer_nbr_of_s_to_acquire = method._device_method().nbr_of_s_to_acquire_ = width;

    // TSR
    method._device_method().TSR_enabled = vm.count( "tsr" );

    // PKD
    method._device_method().pkd_enabled = vm.count( "pkd" );
    method._device_method().pkd_raising_delta = vm[ "raising-delta" ].as< int >();
    method._device_method().pkd_falling_delta = vm[ "falling-delta" ].as< int >();

    if ( method._device_method().TSR_enabled && method._device_method().pkd_enabled ) {
        std::cerr << "TSR & PKD can't be set together\n";
        return 0;
    }
    
#if defined __linux
    signal( SIGINT, &sigint );
    signal( SIGQUIT, &sigint );
    signal( SIGABRT, &sigint );
    signal( SIGHUP, &sigint );
    signal( SIGKILL, &sigint );
#endif

    if ( vm.count( "pkd" ) )
        return pkd_main( method );

    execStatistics::instance().rate_ = vm[ "rate" ].as<double>() * 1.0e-3 * 1.2; // milliseconds -> seconds + 20%

    ppio pp;
    dgpio::pio dgpio;

    if ( ! dgpio.open() )
        std::cerr << "dgpio open failed -- ignored." << std::endl;

    if ( auto md2 = std::make_shared< u5303a::AgMD2 >() ) {

        const char * strInitOptions = "Simulate=false, DriverSetup= Model=U5303A";

        if ( auto p = getenv( "AcqirisOption" ) ) {
            if ( p && std::strcmp( p, "simulate" ) == 0 ) {
                strInitOptions = "Simulate=true, DriverSetup= Model=U5303A";
                simulated = true;
                success = md2->InitWithOptions( "PXI40::0::0::INSTR", VI_TRUE, VI_TRUE, strInitOptions );
            }
        }
        
        if ( !simulated ) {
            for ( auto& res : {
                    "PXI6::0::0::INSTR"
                        , "PXI5::0::0::INSTR"
                        , "PXI4::0::0::INSTR"
                        , "PXI3::0::0::INSTR"
                        , "PXI2::0::0::INSTR"
                        , "PXI1::0::0::INSTR"
                        } ) {

                std::cerr << "Attempting resource: " << res << std::endl;
                if ( ( success = md2->InitWithOptions( res, VI_TRUE, VI_TRUE, strInitOptions ) ) )
                    break;
            }
        }
        
        if ( success ) {

            method._device_method().TSR_enabled = TSR_enabled;
            
            auto ident = std::make_shared< acqrscontrols::u5303a::identify >();
            md2->Identify( ident );

            std::cout << "Identifier:       " << ident->Identifier() << std::endl
                      << "Revision:         " << ident->Revision() << std::endl
                      << "Vendor:           " << ident->Vendor() << std::endl
                      << "Description:      " << ident->Description() << std::endl
                      << "InstrumentModel:  " << ident->InstrumentModel() << std::endl
                      << "Options:          " << ident->Options() << std::endl
                      << "FirmwareRevision: " << ident->FirmwareRevision() << std::endl
                      << "SerialNumber:     " << ident->SerialNumber() << std::endl
                      << "IOVersion:        " << ident->IOVersion() << std::endl
                      << "NbrADCBits:       " << ident->NbrADCBits() << std::endl;

            ViStatus rcode;
            std::string result;

            int32_t count(0);
            md2->GetAttributeViInt32( rcode, "ControlIO", AGMD2_ATTR_CONTROL_IO_COUNT, count );

            ViChar name [ 256 ];
            for ( int i = 1; i <= count; ++i ) {
                rcode = AgMD2_GetControlIOName( md2->session(), i, 256, name );
                if ( rcode == 0 ) {
                    md2->GetAttributeViString( rcode, name, AGMD2_ATTR_CONTROL_IO_SIGNAL, result );
                    std::cout << name << "\t" << result << std::endl;
                    if ( md2->GetAttributeViString( rcode, name, AGMD2_ATTR_CONTROL_IO_AVAILABLE_SIGNALS, result ) )
                        std::cout << "\t" << result << std::endl;
                }
            }
            
            if ( ident->Options().find( "INT" ) != std::string::npos ) // Interleave ON
                md2->ConfigureTimeInterleavedChannelList( "Channel1", "Channel2" );

            md2->setActiveTriggerSource( "External1" );
            
            md2->setTriggerLevel( "External1", method._device_method().ext_trigger_level ); // 1V

            std::cout << "TriggerLevel: " << md2->TriggerLevel( "External1" ) << std::endl;

            md2->setTriggerSlope( "External1", AGMD2_VAL_POSITIVE );

            std::cout << "TriggerSlope: " << md2->TriggerSlope( "External1" ) << std::endl;
            
            double max_rate(0);
            if ( ident->Options().find( "SR1" ) != std::string::npos ) {
                max_rate = ( ident->Options().find( "INT" ) != std::string::npos ) ? 2.0e9 : 1.0e9;
            } else if ( ident->Options().find( "SR2" ) != std::string::npos ) {
                max_rate = ( ident->Options().find( "INT" ) != std::string::npos ) ? 3.2e9 : 1.6e9;
            }
            
            md2->setSampleRate( max_rate );
            method._device_method().samp_rate = md2->SampleRate();
            std::cout << "SampleRate: " << method._device_method().samp_rate << std::endl;            

            md2->setAcquisitionRecordSize( method._device_method().digitizer_nbr_of_s_to_acquire );  // 100us @ 3.2GS/s
            md2->setTriggerDelay( method._device_method().digitizer_delay_to_first_sample );

            md2->setAcquisitionNumRecordsToAcquire( method._device_method().nbr_records );
            md2->setAcquisitionMode( AGMD2_VAL_ACQUISITION_MODE_NORMAL ); // Digitizer mode

            if ( method._device_method().pkd_enabled ) {
                std::cout << "PKD enabled\n";
                pkd_main( md2, method );
                return 0;
            } else {
                md2->setTSREnabled( method._device_method().TSR_enabled );
            }

            md2->CalibrationSelfCalibrate();
            
            std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > > vec;

            const size_t replicates = vm[ "replicates" ].as<int>();

            std::cout << "Replicates: " << replicates << std::endl;
            // size_t deadcount(0);
            // double seconds(0), last(0);
            // size_t dataCount(0);

            std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();            
            
            if ( md2->TSREnabled() ) {
                
                md2->AcquisitionInitiate();
                
                while ( replicates > execStatistics::instance().dataCount_ ) {
                    
                    if ( md2->TSRMemoryOverflowOccured() ) {
                        std::cout << "***** Memory Overflow" << std::endl;
                        break;
                    }

                    while ( !md2->isTSRAcquisitionComplete() )
                        std::this_thread::sleep_for( std::chrono::microseconds( 100 ) ); // assume 1ms trig. interval
                    
                    u5303a::digitizer::readData( *md2, method, vec );
                    md2->TSRContinue();
                    
                    for ( auto& waveform: vec ) {

                        // report if trigger receive interval exceeded
                        double seconds = waveform->meta_.initialXTimeSeconds;

                        if ( std::abs( execStatistics::instance().last_ ) >= std::numeric_limits<double>::epsilon() ) {

                            double interval = execStatistics::instance().difference_from_last( seconds );
                            if ( interval > execStatistics::instance().rate_ )
                                execStatistics::instance().exceededTimings_.push_back( std::make_pair( seconds, interval ) );

                        }
                        execStatistics::instance().last_ = seconds;
                    }
                    
                    execStatistics::instance().dataCount_ += vec.size();
                    
#if defined _MSC_VER
                    const size_t deadsize = 100;
#else
                    constexpr size_t deadsize = 100;
#endif
                    for ( auto& waveform: vec ) {
                        size_t count(0);
                        if ( waveform->isDEAD() )
                            execStatistics::instance().deadCount_++;
                    }
                    vec.clear();  // throw waveforms away.
                }
                
            } else {

                for ( int i = 0; i < replicates; ++i ) {

                    pp << uint8_t( 0x01 );
                    
                    md2->AcquisitionInitiate();
                    md2->AcquisitionWaitForAcquisitionComplete( 3000 );

                    pp << uint8_t( 0x02 );

                    u5303a::digitizer::readData( *md2, method, vec );

                    int protocolIndex = dgpio.protocol_number(); // <- hard wired protocol id
                    execStatistics::instance().dataCount_ += vec.size();

                    if ( __verbose__ >= 5 ) {
                        std::cout << "u5303a::digitizer::readData read " << vec.size() << " waveform(s), proto#"
                                  << dgpio.protocol_number()
                                  << "\t(" << i << "/" << replicates << ")" << execStatistics::instance().dataCount_ << std::endl;
                    }

                    vec.clear();
                }
            }

            std::cout << execStatistics::instance();
            
        }
    }
    
    return 0;
}

constexpr ViInt64 const numRecords = 1;			// only record=1 is supported in PKD mode
ViInt32 const numAverages = 32;			// accumulation from 1 to 520'000 triggers 
ViInt64 const recordSize = 1600;		// record size from 1 up to 240 KSamples
ViReal64 const range = 1.0;				// range: 1 or 2 V 
ViReal64 const offset = 0.0;			// offset: ｱ2 x FSR
ViReal64 const trigLevel = 0.0;			// trigger level. External trigger configured in the code. 
ViInt32 const risingDelta = 0x100;		// Rising Delta: from 1 to 8192 LSB
ViInt32 const fallingDelta = 0x100;		// Falling Delta: from 1 to 8192 LSB

void
pkd_main( std::shared_ptr< u5303a::AgMD2 > md2, const acqrscontrols::u5303a::method& m )
{
    using u5303a::AgMD2;
    
    AgMD2::log( md2->setTSREnabled( false ), __FILE__, __LINE__ );

    AgMD2::log( md2->setAcquisitionNumberOfAverages( m._device_method().nbr_of_averages ), __FILE__, __LINE__ ); // 150

    AgMD2::log( md2->setAttributeViInt64( "", AGMD2_ATTR_NUM_RECORDS_TO_ACQUIRE, 1 ), __FILE__, __LINE__ ); // 145

    // Enable the Peak Detection mode 	
    AgMD2::log( md2->setAttributeViInt32( "", AGMD2_ATTR_ACQUISITION_MODE, AGMD2_VAL_ACQUISITION_MODE_PEAK_DETECTION ), __FILE__, __LINE__); // 153
    

    // Configure the data inversion mode - VI_FALSE (no data inversion) by default
    AgMD2::log( md2->setAttributeViBoolean( "Channel1"
                                            , AGMD2_ATTR_CHANNEL_DATA_INVERSION_ENABLED
                                            , m._device_method().invert_signal ? VI_TRUE : VI_FALSE ), __FILE__,__LINE__);  // 157
    
    // Configure the accumulation enable mode: the peak value is stored (VI_TRUE) or the peak value is forced to '1' (VI_FALSE).
    AgMD2::log( md2->setAttributeViBoolean( "Channel1"
                                            , AGMD2_ATTR_PEAK_DETECTION_AMPLITUDE_ACCUMULATION_ENABLED
                                            , m._device_method().pkd_amplitude_accumulation_enabled ? VI_TRUE : VI_FALSE ), __FILE__,__LINE__); // 160

    // Configure the RisingDelta and FallingDelta in LSB: define the amount by which two consecutive samples must differ to be
    // considered as rising/falling edge in the peak detection algorithm.
    AgMD2::log( md2->setAttributeViInt32( "Channel1", AGMD2_ATTR_PEAK_DETECTION_RISING_DELTA, m._device_method().pkd_raising_delta ), __FILE__,__LINE__ ); // 163
    
    AgMD2::log( md2->setAttributeViInt32( "Channel1", AGMD2_ATTR_PEAK_DETECTION_FALLING_DELTA, m._device_method().pkd_falling_delta ), __FILE__,__LINE__); // 164
    
    AgMD2::log( md2->setAcquisitionRecordSize( m._device_method().nbr_of_s_to_acquire_ ), __FILE__,__LINE__);
    AgMD2::log( md2->setAcquisitionNumRecordsToAcquire( 1 ), __FILE__,__LINE__);
    AgMD2::log( md2->setAcquisitionNumberOfAverages( m._device_method().nbr_of_averages ), __FILE__, __LINE__ );

    std::cout << "Performing self-calibration\n";
    AgMD2::log( md2->CalibrationSelfCalibrate(), __FILE__,__LINE__ );

    ViInt64 arraySize = 0;
    
    u5303a::AgMD2::log( AgMD2_QueryMinWaveformMemory( md2->session(), 16, numRecords, 0, recordSize, &arraySize ), __FILE__, __LINE__ );
    
    // Perform the acquisition.
    ViInt32 const timeoutInMs = 2000;
    std::cout << "Performing acquisition\n";

    AgMD2::log( md2->AcquisitionInitiate(), __FILE__,__LINE__);
    
    AgMD2::log( md2->AcquisitionWaitForAcquisitionComplete( 3000 ), __FILE__,__LINE__ );

    // u5303a::digitizer::readData( *md2, m, vec );
    
    std::cout << "Acquisition completed\n";

    struct data {
        ViInt32 actualAverages;
        ViInt64 actualRecords;
        ViInt64 waveformArrayActualSize;
        ViInt64 actualPoints[numRecords];
        ViInt64 firstValidPoint[numRecords];
        ViReal64 initialXOffset;
        ViReal64 initialXTimeSeconds[numRecords], initialXTimeFraction[numRecords];
        ViReal64 xIncrement, scaleFactor, scaleOffset;
        ViInt32 flags[numRecords];
    };
    data d1 = { 0 }, d2 = { 0 };
    std::vector<ViInt32> dataArray1( arraySize ), dataArray2( arraySize );

	// Read the peaks on Channel 1 in INT32.
    AgMD2::log( AgMD2_FetchAccumulatedWaveformInt32( md2->session(),  "Channel1",
                                                     0, numRecords, 0, recordSize, arraySize, dataArray1.data(),
                                                     &d1.actualAverages, &d1.actualRecords, d1.actualPoints, d1.firstValidPoint,
                                                     &d1.initialXOffset, d1.initialXTimeSeconds, d1.initialXTimeFraction,
                                                     &d1.xIncrement, &d1.scaleFactor, &d1.scaleOffset, d1.flags )
                , __FILE__, __LINE__ );

    // Read the averaged waveform on Channel 2 in INT32.

    AgMD2::log( AgMD2_FetchAccumulatedWaveformInt32( md2->session(), "Channel2",
                                                     0, numRecords, 0, recordSize, arraySize, dataArray2.data(),
                                                     &d2.actualAverages, &d2.actualRecords, d2.actualPoints, d2.firstValidPoint,
                                                     &d2.initialXOffset, d2.initialXTimeSeconds, d2.initialXTimeFraction,
                                                     &d2.xIncrement, &d2.scaleFactor, &d2.scaleOffset, d2.flags )
                , __FILE__, __LINE__ );
    
    std::cout << "\nactualAverages: " << d1.actualAverages;

    // Read the peaks values on Channel 1 
    std::cout << "\nProcessing data\n";

    constexpr size_t currentRecord = 0;
    
    for ( size_t currentPoint = 0;
          currentPoint < d1.actualPoints[currentRecord] && currentPoint < d2.actualPoints[currentRecord]; ++currentPoint )  {

        ViInt32 valuePKD = dataArray1[d1.firstValidPoint[currentRecord] + currentPoint];
        ViInt32 valueAVG = dataArray2[d2.firstValidPoint[currentRecord] + currentPoint];

        std::cout << valuePKD << "\t" << valueAVG << std::endl;
    }

    std::cout << "\nProcessing completed\n";
}

int
pkd_main( const acqrscontrols::u5303a::method& )
{
    using u5303a::AgMD2;
    
    std::cout << "PeakDetection + Averager POC\n\n";
    ViSession session = 0;
    ViBoolean const idQuery = VI_FALSE;
    ViBoolean const reset   = VI_TRUE;
    FILE *pFile;
    pFile = fopen("data.txt", "w");

    char resource[] = "PXI5::0::0::INSTR";
    char options[] = "Simulate=false, DriverSetup= Model=U5303A";

    // Initialize the driver. See driver help topic "Initializing the IVI-C Driver" for additional information.
    AgMD2::log( AgMD2_InitWithOptions( resource, idQuery, reset, options, &session ), __FILE__,__LINE__ );

    std::cout << "Driver initialized \n";
    
    // Abort execution if instrument is still in simulated mode.
    ViBoolean simulate;
    AgMD2::log( AgMD2_GetAttributeViBoolean( session, "", AGMD2_ATTR_SIMULATE, &simulate ), __FILE__,__LINE__ );

    if ( simulate == VI_TRUE )
    {
        std::cout << "The PeakDetection features are not supported in simulated mode.";
        std::cout << "\nPlease update the resource string (resource[]) to match your configuration, and update the init options string (options[]) to disable simulation.";
        
        return -1;
    }

    // Check the instrument contains the required PKD module option.
    ViChar str[128] = {'\0'};
    AgMD2::log( AgMD2_GetAttributeViString( session, "", AGMD2_ATTR_INSTRUMENT_INFO_OPTIONS, sizeof( str ), str ), __FILE__,__LINE__ );
    if ( std::string( str ).find( "PKD" ) == std::string::npos )
    {
        std::cout << "The required PKD module option is missing from the instrument.";
        return -1;
    }

    // Read and output a few attributes.
    AgMD2::log( AgMD2_GetAttributeViString( session, "", AGMD2_ATTR_SPECIFIC_DRIVER_PREFIX, sizeof( str ), str ), __FILE__,__LINE__ );
    std::cout << "Driver prefix:      " << str << '\n';
    fprintf(pFile, "Driver prefix:      %s\n", str);
    AgMD2::log( AgMD2_GetAttributeViString( session, "", AGMD2_ATTR_SPECIFIC_DRIVER_REVISION, sizeof( str), str ), __FILE__,__LINE__ );
    std::cout << "Driver revision:    " << str << '\n';
    fprintf(pFile, "Driver revision:    %s\n", str);
    AgMD2::log( AgMD2_GetAttributeViString( session, "", AGMD2_ATTR_SPECIFIC_DRIVER_VENDOR, sizeof( str ), str ), __FILE__,__LINE__ );
    std::cout << "Driver vendor:      " << str << '\n';
    fprintf(pFile, "Driver revision:    %s\n", str);
    AgMD2::log( AgMD2_GetAttributeViString( session, "", AGMD2_ATTR_SPECIFIC_DRIVER_DESCRIPTION, sizeof( str ), str ), __FILE__,__LINE__ );
    std::cout << "Driver description: " << str << '\n';
    fprintf(pFile, "Driver description: %s\n", str);
   AgMD2::log( AgMD2_GetAttributeViString( session, "", AGMD2_ATTR_INSTRUMENT_MODEL,                     sizeof(str), str), __FILE__,__LINE__ );
    std::cout << "Instrument model:   " << str << "\n";
    fprintf(pFile, "Instrument model:   %s\n", str);
    AgMD2::log( AgMD2_GetAttributeViString( session, "", AGMD2_ATTR_INSTRUMENT_INFO_OPTIONS,              sizeof(str), str), __FILE__,__LINE__ );
    std::cout << "Instrument options: " << str << '\n';
    fprintf(pFile, "Instrument options: %s\n", str);
    AgMD2::log( AgMD2_GetAttributeViString( session, "", AGMD2_ATTR_INSTRUMENT_FIRMWARE_REVISION,         sizeof(str), str ), __FILE__,__LINE__ );
    std::cout << "Firmware revision:  " << str << "\n";
    fprintf(pFile, "Firmware revision:  %s\n", str);
    AgMD2::log( AgMD2_GetAttributeViString( session, "", AGMD2_ATTR_INSTRUMENT_INFO_SERIAL_NUMBER_STRING, sizeof(str), str ), __FILE__,__LINE__ );
    std::cout << "Serial number:      " << str << "\n";
    fprintf(pFile, "Serial number:      %s\n", str);
 
    std::cout << "\nSimulate:           " << ( simulate?"True":"False" ) << '\n';

    // Configure the acquisition.
     ViInt32 const coupling = AGMD2_VAL_VERTICAL_COUPLING_DC;
    std::cout << "Configuring acquisition\n";
	// Configure channel 1
    std::cout << "Range:              " << range << '\n';
    std::cout << "Offset:             " << offset << '\n';
    std::cout << "Coupling:           " << ( coupling?"DC":"AC" ) << '\n';
    AgMD2::log( AgMD2_ConfigureChannel( session, "Channel1", range, offset, coupling, VI_TRUE ), __FILE__,__LINE__ );

	// Configure the number of records (only 1 record is supported in AVG+PKD) and record size (in number of samples)
    std::cout << "Number of records:  " << numRecords << '\n';
    std::cout << "Record size:        " << recordSize << '\n';
    AgMD2::log( AgMD2_SetAttributeViInt64( session, "", AGMD2_ATTR_NUM_RECORDS_TO_ACQUIRE, numRecords ), __FILE__,__LINE__ );
    AgMD2::log( AgMD2_SetAttributeViInt64( session, "", AGMD2_ATTR_RECORD_SIZE, recordSize ), __FILE__,__LINE__ );

	// Configure the number of accumulation
    std::cout << "Number of averages: " << numAverages << "\n\n";
    AgMD2::log( AgMD2_SetAttributeViInt32( session, "", AGMD2_ATTR_ACQUISITION_NUMBER_OF_AVERAGES, numAverages ), __FILE__,__LINE__ );
 
	// Enable the Peak Detection mode 	
    AgMD2::log( AgMD2_SetAttributeViInt32( session, "", AGMD2_ATTR_ACQUISITION_MODE, AGMD2_VAL_ACQUISITION_MODE_PEAK_DETECTION ), __FILE__,__LINE__ );

	// Configure the peak detection on channel 1
	// Configure the data inversion mode - VI_FALSE (no data inversion) by default
    AgMD2::log( AgMD2_SetAttributeViBoolean( session, "Channel1", AGMD2_ATTR_CHANNEL_DATA_INVERSION_ENABLED, VI_FALSE ), __FILE__,__LINE__ );

	// Configure the accumulation enable mode: the peak value is stored (VI_TRUE) or the peak value is forced to '1' (VI_FALSE).
	AgMD2::log( AgMD2_SetAttributeViBoolean( session, "Channel1", AGMD2_ATTR_PEAK_DETECTION_AMPLITUDE_ACCUMULATION_ENABLED, VI_FALSE ), __FILE__,__LINE__ );
	
	// Configure the RisingDelta and FallingDelta in LSB: define the amount by which two consecutive samples must differ to be considered as rising/falling edge in the peak detection algorithm.
	AgMD2::log( AgMD2_SetAttributeViInt32( session, "Channel1", AGMD2_ATTR_PEAK_DETECTION_RISING_DELTA, risingDelta ), __FILE__,__LINE__ );
    AgMD2::log( AgMD2_SetAttributeViInt32( session, "Channel1", AGMD2_ATTR_PEAK_DETECTION_FALLING_DELTA, fallingDelta ), __FILE__,__LINE__ );

    // Configure the trigger.
    std::cout << "Configuring trigger\n";
    AgMD2::log( AgMD2_SetAttributeViString( session, "", AGMD2_ATTR_ACTIVE_TRIGGER_SOURCE, "External1" ), __FILE__,__LINE__ );
    AgMD2::log( AgMD2_SetAttributeViReal64( session, "External1", AGMD2_ATTR_TRIGGER_LEVEL, trigLevel ), __FILE__,__LINE__ );
    // Calibrate the instrument.
    std::cout << "Performing self-calibration\n";
    AgMD2::log( AgMD2_SelfCalibrate( session ), __FILE__,__LINE__ );
 
    // Perform the acquisition.
    ViInt32 const timeoutInMs = 2000;
    std::cout << "Performing acquisition\n";
    AgMD2::log( AgMD2_InitiateAcquisition( session ), __FILE__,__LINE__ );
    AgMD2::log( AgMD2_WaitForAcquisitionComplete( session, timeoutInMs ), __FILE__,__LINE__ );
    std::cout << "Acquisition completed\n";

    // Fetch the acquired data in array.
    ViInt64 arraySize = 0;
    AgMD2::log( AgMD2_QueryMinWaveformMemory( session, 32, numRecords, 0, recordSize, &arraySize ), __FILE__,__LINE__ );

    std::vector<ViInt32> dataArray( arraySize );
    ViInt32 actualAverages = 0;
    ViInt64 actualRecords = 0, waveformArrayActualSize = 0;
    ViInt64 actualPoints[numRecords], firstValidPoint[numRecords];
    ViReal64 initialXOffset, initialXTimeSeconds[numRecords], initialXTimeFraction[numRecords];
    ViReal64 xIncrement = 0.0, scaleFactor = 0.0, scaleOffset = 0.0;
    ViInt32 flags[numRecords];

	// Read the peaks on Channel 1 in INT32.
	AgMD2::log( AgMD2_FetchAccumulatedWaveformInt32( session, "Channel1",
                                                     0, numRecords, 0, recordSize, arraySize, &dataArray[0],
                                                     &actualAverages, &actualRecords, actualPoints, firstValidPoint,
                                                     &initialXOffset, initialXTimeSeconds, initialXTimeFraction,
                                                     &xIncrement, &scaleFactor, &scaleOffset, flags ), __FILE__,__LINE__ );

    std::cout << "\nactualAverages: " << actualAverages;
    fprintf(pFile, "\nactualAverages: %d", actualAverages);

    // Read the peaks values on Channel 1 
    std::cout << "\nProcessing data\n";
    fprintf(pFile, "\nPeaks values ");
    for ( int currentRecord = 0; currentRecord < actualRecords; ++currentRecord )
    {
        for ( int currentPoint = 0; currentPoint < actualPoints[currentRecord]; ++currentPoint )
        {
            ViInt32 valuePKD = dataArray[firstValidPoint[currentRecord] + currentPoint];
 	        fprintf(pFile, "\n%d", valuePKD);
       }
    }


   // Read the averaged waveform on Channel 2 in INT32.
    AgMD2::log( AgMD2_FetchAccumulatedWaveformInt32( session, "Channel2",
                                                     0, numRecords, 0, recordSize, arraySize, &dataArray[0],
                                                     &actualAverages, &actualRecords, actualPoints, firstValidPoint,
                                                     &initialXOffset, initialXTimeSeconds, initialXTimeFraction,
                                                     &xIncrement, &scaleFactor, &scaleOffset, flags ), __FILE__,__LINE__ );

   fprintf(pFile, "\nAveraged data in LSB");
   for ( int currentRecord = 0; currentRecord < actualRecords; ++currentRecord )
   {
        for ( int currentPoint = 0; currentPoint < actualPoints[currentRecord]; ++currentPoint )
        {
           ViInt32 valueAVG = dataArray[firstValidPoint[currentRecord] + currentPoint];
  		   fprintf(pFile, "\n%d", valueAVG);
        }
   }

    std::cout << "\nProcessing completed\n";

    fclose (pFile);
     // Close the driver.
    AgMD2::log( AgMD2_close( session ), __FILE__,__LINE__ );
    std::cout << "Driver closed\n";

    return 0;
}

