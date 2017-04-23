/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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
            ( "records,r",  po::value<int>()->default_value( 1 ), "Number of records" )
            ( "average,a",  po::value<int>()->default_value( 0 ), "Number of average" )
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
                success = md2->InitWithOptions( "PXI40::0::0::INSTR", VI_TRUE, VI_TRUE, strInitOptions );
            }
        }
        
        if ( !simulated ) {
            for ( auto& res : { "PXI5::0::0::INSTR", "PXI4::0::0::INSTR", "PXI3::0::0::INSTR", "PXI2::0::0::INSTR", "PXI1::0::0::INSTR" } ) {
                std::cout << "Initialize resource: " << res;
                if ( success = md2->InitWithOptions( res, VI_TRUE, VI_TRUE, strInitOptions ) )
                    break;
            }
        }
        
        if ( success ) {

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
            method._device_method().delay_to_first_sample_ = method._device_method().digitizer_delay_to_first_sample = vm[ "delay" ].as<double>() * 1.0e-6;

            // data length
            uint32_t width = uint32_t( ( vm[ "width" ].as<double>() * 1.0e-6 ) * method._device_method().samp_rate + 0.5 );
            method._device_method().digitizer_nbr_of_s_to_acquire = method._device_method().nbr_of_s_to_acquire_ = width;

            // TSR
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

            md2->setTSREnabled( method._device_method().TSR_enabled );            

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
                        std::cout << "u5303a::digitizer::readData read " << vec.size() << " waveforms @ protocol#"
                                  << dgpio.protocol_number()
                                  << "\ttotal: " << execStatistics::instance().dataCount_ << std::endl;
                    }

                    vec.clear();
                }
            }

            std::cout << execStatistics::instance();
            
        }
    }
    
    return 0;
}
