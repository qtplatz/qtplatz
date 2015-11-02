/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <u5303a/ppio.hpp>
#include <u5303a/agmd2.hpp>
#include <u5303a/digitizer.hpp>
#include <acqrscontrols/u5303a/identify.hpp>
#include <acqrscontrols/u5303a/method.hpp>
#include <boost/program_options.hpp>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

namespace po = boost::program_options;

int
main( int argc, char * argv [] )
{
    bool success( false ), simulated( false );

    po::variables_map vm;
    po::options_description description( "test_u5303a" );
    {
        description.add_options()
            ( "help,h",    "Display this help message" )
            ( "tsr,s",      po::value<int>()->default_value( 0 ), "TSR enable(1)/disable(0)" )
            ( "records,r",  po::value<int>()->default_value( 1 ), "Number of records" )
            ( "average,a",  po::value<int>()->default_value( 0 ), "Number of average" )
            ( "mode,m",     po::value<int>()->default_value( 0 ), "Digitizer (0) or Averager (2)" )
            ( "delay,d",    po::value<double>()->default_value( 0.0 ), "Delay (us)" )
            ( "width,w",    po::value<double>()->default_value( 100.0 ), "Waveform width (us)" )
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }
    
    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    ppio pp;

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
            for ( auto& res : { "PXI4::0::0::INSTR", "PXI3::0::0::INSTR", "PXI2::0::0::INSTR", "PXI1::0::0::INSTR" } ) {
                std::cout << "Initialize resource: " << res;
                if ( success = md2->InitWithOptions( res, VI_TRUE, VI_TRUE, strInitOptions ) )
                    break;
            }
        }

        
        
        if ( success ) {

            acqrscontrols::u5303a::method method;
            method.channels_ = 0x01;
            method.mode_ = vm[ "mode" ].as<int>();
            method.method_.front_end_range = 1.0;  // V
            method.method_.front_end_offset = 0.0; // V
            method.method_.ext_trigger_level = 1.0;
            method.method_.samp_rate = 3.2e9;

            // MultiRecords or single record
            method.method_.nbr_records = vm[ "records" ].as<int>();
            method.method_.nbr_of_averages = vm[ "average" ].as<int>();    // 0 for digitizer

            // delay
            method.method_.delay_to_first_sample_ = method.method_.digitizer_delay_to_first_sample = vm[ "delay" ].as<double>() * 1.0e-6;

            // data length
            uint32_t width = uint32_t( ( vm[ "width" ].as<double>() * 1.0e-6 ) * method.method_.samp_rate + 0.5 );
            method.method_.digitizer_nbr_of_s_to_acquire = method.method_.nbr_of_s_to_acquire_ = width;

            // TSR
            method.method_.TSR_enabled = vm[ "tsr" ].as<int>() ? true : false;
            
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
            
            if ( ident->Options().find( "INT" ) != std::string::npos )   // Interleave ON
                md2->ConfigureTimeInterleavedChannelList( "Channel1", "Channel2" );

            md2->setActiveTriggerSource( "External1" );
            
            md2->setTriggerLevel( "External1", method.method_.ext_trigger_level ); // 1V

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
            method.method_.samp_rate = md2->SampleRate();
            std::cout << "SampleRate: " << method.method_.samp_rate << std::endl;            

            md2->setAcquisitionRecordSize( method.method_.digitizer_nbr_of_s_to_acquire );  // 100us @ 3.2GS/s
            md2->setTriggerDelay( method.method_.digitizer_delay_to_first_sample );

            md2->setAcquisitionNumRecordsToAcquire( method.method_.nbr_records );
            md2->setAcquisitionMode( AGMD2_VAL_ACQUISITION_MODE_NORMAL ); // Digitizer mode

            md2->setTSREnabled( method.method_.TSR_enabled );            

            md2->CalibrationSelfCalibrate();
            
            std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();

            std::shared_ptr< acqrscontrols::u5303a::waveform > w1;
            
            std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > > vec;

            constexpr size_t replicates = 1000;
            
            std::cout << "********** TSR " << std::boolalpha << md2->TSREnabled() << std::endl;

            double seconds(0), last(0);

            if ( md2->TSREnabled() ) {
                
                md2->AcquisitionInitiate();

                size_t dataCount(0);

                while ( replicates > dataCount ) {
                    
                    if ( md2->TSRMemoryOverflowOccured() ) {
                        std::cout << "Memory Overflow" << std::endl;
                        break;
                    }

                    while ( !md2->isTSRAcquisitionComplete() )
                        std::this_thread::sleep_for( std::chrono::microseconds( 100 ) ); // assume 1ms trig. interval
                    
                    u5303a::digitizer::readData( *md2, method, vec );
                    md2->TSRContinue();
                    
                    std::cout << "read : " << vec.size() << std::endl;
                    
                    for ( auto& waveform: vec ) {
                        seconds = waveform->meta_.initialXTimeSeconds;
                        if ( last != 0 )
                            std::cout << seconds << ", " << seconds - last << std::endl;
                        last = seconds;
                    }
                    

                    dataCount += vec.size();

                    vec.clear();  // delete data
                }
                    
            } else {

                for ( int i = 0; i < replicates; ++i ) {

                    pp << uint8_t( 0x01 );

                    md2->AcquisitionInitiate();
                    md2->AcquisitionWaitForAcquisitionComplete( 1000 );

                    pp << uint8_t( 0x02 );

                    u5303a::digitizer::readData( *md2, method, vec );
                    vec.clear();
                }
            }
            
            uint64_t ns = std::chrono::duration_cast< std::chrono::nanoseconds >( std::chrono::steady_clock::now() - tp ).count();
            double s = double(ns) * 1.0e-9;
            std::cout << "Took " << s << " seconds; " << double(replicates) / s << "Hz" << std::endl;
        }
        
    }
    
    return 0;
}
