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
#include <boost/format.hpp>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <ostream>
#include <iostream>
#include <memory>
#include <ratio>
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

int pkd_main( std::shared_ptr< u5303a::AgMD2 >, const acqrscontrols::u5303a::method&, size_t replicates );

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
            ( "invert-signal", po::value<bool>()->default_value( true ), "Invert signal {true/false}" )
            ( "raising-delta", po::value<int>()->default_value( 20 ), "PKD Raising delta" )
            ( "falling-delta", po::value<int>()->default_value( 20 ), "PKD Falling delta" )
            ( "mode,m",     po::value<int>()->default_value( 0 ), "=0 Normal(digitizer); =2 Averager" )
            ( "delay,d",    po::value<double>()->default_value( 0.0 ), "Delay (us)" )
            ( "width,w",    po::value<double>()->default_value( 50.0 ), "Waveform width (us)" )
            ( "replicates", po::value<int>()->default_value( 1000 ), "Number of triggers to acquire waveforms" )
            ( "rate",       po::value<double>()->default_value( 1.0 ),  "Expected trigger interval in millisecond (trigger drop/nodrop validation)" )
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
    method._device_method().samp_rate = vm.count( "pkd" ) ? 1.6e9 : 3.2e9;
    method._device_method().invert_signal = vm[ "invert-signal" ].as< bool >();

    // MultiRecords or single record
    method._device_method().nbr_records = vm[ "records" ].as<int>();
    method._device_method().nbr_of_averages = vm[ "average" ].as<int>();    // 0 for digitizer

    if ( vm.count( "pkd" ) && method._device_method().nbr_of_averages == 0 )
        method._device_method().nbr_of_averages = 2;

    if ( ( method._device_method().nbr_of_averages > 8 ) && ( method._device_method().nbr_of_averages % 8 ) ) {
        std::cout << "Number of waveforms to be averaged must be fold of 8 or less than 8" << std::endl;
        method._device_method().nbr_of_averages &= ~07;
    }

    // delay (seconds)
    method._device_method().delay_to_first_sample_
        = method._device_method().digitizer_delay_to_first_sample
        = vm[ "delay" ].as<double>() / std::micro::den;

    // data length
    uint32_t width = uint32_t( ( vm[ "width" ].as<double>() / std::micro::den ) * method._device_method().samp_rate + 0.5 );
    method._device_method().digitizer_nbr_of_s_to_acquire = method._device_method().nbr_of_s_to_acquire_ = width;

    std::cout << "width=" << width << std::endl;

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

    std::cout << "nbr_of_averages = " << method._device_method().nbr_of_averages << std::endl;

    const size_t replicates = vm[ "replicates" ].as<int>();
        
#if defined __linux
    signal( SIGINT, &sigint );
    signal( SIGQUIT, &sigint );
    signal( SIGABRT, &sigint );
    signal( SIGHUP, &sigint );
    signal( SIGKILL, &sigint );
#endif

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
                success = md2->InitWithOptions( "PXI40::0::0::INSTR", VI_FALSE, VI_TRUE, strInitOptions );
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
                if ( ( success = md2->InitWithOptions( res, VI_FALSE, VI_TRUE, strInitOptions ) ) )
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

            md2->setActiveTriggerSource( "External1" );
            md2->setTriggerLevel( "External1", method._device_method().ext_trigger_level ); // 1V
            std::cout << "TriggerLevel: " << md2->TriggerLevel( "External1" ) << std::endl;
            md2->setTriggerSlope( "External1", AGMD2_VAL_POSITIVE );
            std::cout << "TriggerSlope: " << md2->TriggerSlope( "External1" ) << std::endl;

            if ( vm.count( "pkd" ) && ident->Options().find( "PKD" ) != std::string::npos )
                return pkd_main( md2, method, replicates );
            
            if ( ident->Options().find( "INT" ) != std::string::npos ) // Interleave ON
                md2->ConfigureTimeInterleavedChannelList( "Channel1", "Channel2" );

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

            md2->setTSREnabled( method._device_method().TSR_enabled );

            md2->CalibrationSelfCalibrate();
            
            std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > > vec;

            std::cout << "Replicates: " << replicates << std::endl;

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

int
pkd_main( std::shared_ptr< u5303a::AgMD2 > md2, const acqrscontrols::u5303a::method& m, size_t replicates )
{
    using u5303a::AgMD2;
    
    std::cout << "PeakDetection + Averager POC\n\n";
    std::cout << "Driver initialized \n";
    
    // Configure the acquisition.
    
    ViInt32 const coupling = AGMD2_VAL_VERTICAL_COUPLING_DC;
    std::cerr << "Configuring acquisition\n";
    std::cerr << "Range:              " << m._device_method().front_end_range << '\n';
    std::cerr << "Offset:             " << m._device_method().front_end_offset << '\n';
    std::cerr << "Coupling:           " << ( coupling?"DC":"AC" ) << '\n';

    AgMD2::log( AgMD2_ConfigureChannel( md2->session(), "Channel1"
                                        , m._device_method().front_end_range
                                        , m._device_method().front_end_offset, coupling, VI_TRUE ), __FILE__,__LINE__ );

	// Configure the number of records (only 1 record is supported in AVG+PKD) and record size (in number of samples)
    std::cout << "Number of records:  " << numRecords << '\n';
    std::cout << "Record size:        " << m._device_method().digitizer_nbr_of_s_to_acquire << '\n';
    md2->setAttributeViInt64( "", AGMD2_ATTR_NUM_RECORDS_TO_ACQUIRE, numRecords );

    md2->setAcquisitionRecordSize( m._device_method().digitizer_nbr_of_s_to_acquire );
    md2->setTriggerDelay( m._device_method().digitizer_delay_to_first_sample );

	// Configure the number of accumulation
    std::cout << "Number of averages: " << m._device_method().nbr_of_averages << "\n\n";
    md2->setAttributeViInt32( "", AGMD2_ATTR_ACQUISITION_NUMBER_OF_AVERAGES, m._device_method().nbr_of_averages );
 
	// Enable the Peak Detection mode 	
    md2->setAttributeViInt32( "", AGMD2_ATTR_ACQUISITION_MODE, AGMD2_VAL_ACQUISITION_MODE_PEAK_DETECTION );

	// Configure the peak detection on channel 1
	// Configure the data inversion mode - VI_FALSE (no data inversion) by default
    md2->setAttributeViBoolean( "Channel1"
                                , AGMD2_ATTR_CHANNEL_DATA_INVERSION_ENABLED
                                , m._device_method().invert_signal ? VI_TRUE : VI_FALSE );
    std::cout << "Signal inversion: " << std::boolalpha << m._device_method().invert_signal << std::endl;

	md2->setAttributeViBoolean( "Channel1"
                                , AGMD2_ATTR_PEAK_DETECTION_AMPLITUDE_ACCUMULATION_ENABLED
                                , m._device_method().pkd_amplitude_accumulation_enabled ? VI_TRUE : VI_FALSE );

	md2->setAttributeViInt32( "Channel1", AGMD2_ATTR_PEAK_DETECTION_RISING_DELTA, m._device_method().pkd_raising_delta );
    md2->setAttributeViInt32( "Channel1", AGMD2_ATTR_PEAK_DETECTION_FALLING_DELTA, m._device_method().pkd_falling_delta );

    // Configure the trigger.
    std::cout << "Configuring trigger\n";
    md2->setActiveTriggerSource( "External1" );
    md2->setTriggerLevel( "External1", m._device_method().ext_trigger_level ); // 1V

    std::cout << "Performing self-calibration\n";
    md2->CalibrationSelfCalibrate();

    // Perform the acquisition.
    ViInt32 const timeoutInMs = 2000;
    std::cout << "Performing acquisition\n";

    md2->AcquisitionInitiate();
    md2->AcquisitionWaitForAcquisitionComplete( 3000 );

    std::cout << "Acquisition completed\n";

    // Fetch the acquired data in array.
    ViInt64 arraySize = 0;
    AgMD2::log( AgMD2_QueryMinWaveformMemory( md2->session(), 32, numRecords, 0, m._device_method().nbr_of_s_to_acquire_, &arraySize ), __FILE__,__LINE__ );

    struct data {
        ViInt32 actualAverages;
        ViInt64 actualRecords;
        ViInt64 waveformArrayActualSize;
        ViInt64 actualPoints[numRecords];
        ViInt64 firstValidPoint[numRecords];
        ViReal64 initialXOffset;
        ViReal64 initialXTimeSeconds[numRecords];
        ViReal64 initialXTimeFraction[numRecords];
        ViReal64 xIncrement, scaleFactor, scaleOffset;
        ViInt32 flags[numRecords];
        void print( std::ostream& o, const char * heading ) const {
            std::cout << heading << ":\t"
                      << boost::format( "actualAverages: %d\tactualPoints\%d\tfirstValidPoint%d" )
                % actualAverages % actualPoints[0] % firstValidPoint[0]
                      << boost::format( "\tinitialXOffset: %d\tinitialXTime: %g" )
                % initialXOffset % ( initialXTimeSeconds[0] + initialXTimeFraction[0] )
                      << boost::format( "\txIncrement: %d\tscaleFactor: %g\tscaleOffset: %g\tflags: 0x%x" )
                % xIncrement % scaleFactor % scaleOffset % flags[0]
                      << std::endl;
        }

        inline double time( size_t idx ) const {
            return initialXOffset + idx * xIncrement;
        }
        inline double toVolts( int value ) const {
            return double(value) * scaleFactor + scaleOffset;
        }
    };

    data d1 = { 0 }, d2 = { 0 };
    std::vector<ViInt32> dataArray1( arraySize ), dataArray2( arraySize );

    while ( replicates-- ) {
        // Read the peaks on Channel 1 in INT32.
        AgMD2::log( AgMD2_FetchAccumulatedWaveformInt32( md2->session(),  "Channel1",
                                                         0, numRecords, 0, m._device_method().nbr_of_s_to_acquire_, arraySize, dataArray1.data(),
                                                         &d1.actualAverages, &d1.actualRecords, d1.actualPoints, d1.firstValidPoint,
                                                         &d1.initialXOffset, d1.initialXTimeSeconds, d1.initialXTimeFraction,
                                                         &d1.xIncrement, &d1.scaleFactor, &d1.scaleOffset, d1.flags )
                    , __FILE__, __LINE__ );

        // Read the averaged waveform on Channel 2 in INT32.

        AgMD2::log( AgMD2_FetchAccumulatedWaveformInt32( md2->session(), "Channel2",
                                                         0, numRecords, 0, m._device_method().nbr_of_s_to_acquire_, arraySize, dataArray2.data(),
                                                         &d2.actualAverages, &d2.actualRecords, d2.actualPoints, d2.firstValidPoint,
                                                         &d2.initialXOffset, d2.initialXTimeSeconds, d2.initialXTimeFraction,
                                                         &d2.xIncrement, &d2.scaleFactor, &d2.scaleOffset, d2.flags )
                    , __FILE__, __LINE__ );
    
        std::cout << "\nactualAverages: " << d1.actualAverages;

        // Read the peaks values on Channel 1 
        std::cout << "\nProcessing data\n";

        d1.print( std::cout, "Channel1(PKD)" );
        d2.print( std::cout, "Channel2(PKD)" );

        constexpr size_t currentRecord = 0;
    
        for ( size_t currentPoint = 0;
              currentPoint < d1.actualPoints[currentRecord] && currentPoint < d2.actualPoints[currentRecord]; ++currentPoint )  {

            ViInt32 valuePKD = dataArray1[d1.firstValidPoint[currentRecord] + currentPoint];
            ViInt32 valueAVG = dataArray2[d2.firstValidPoint[currentRecord] + currentPoint];

            std::cout << boost::format("%.7le\t%d\t%.5lf\t|\t%d\t%.5lf\n")
                % d1.time( currentPoint ) % valuePKD % d1.toVolts( valuePKD ) % valueAVG % d2.toVolts( valueAVG );
        }
    }
    std::cout << "\nProcessing completed\n";

    /////////////////////////

    md2.reset();
    std::cout << "Driver closed\n";

    return 0;
}

