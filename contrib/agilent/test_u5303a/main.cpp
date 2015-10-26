
#include <u5303a/agmd2.hpp>
#include <u5303a/digitizer.hpp>
#include <acqrscontrols/u5303a/identify.hpp>
#include <acqrscontrols/u5303a/method.hpp>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <chrono>

ViChar resource[] = "PXI40::0::0::INSTR";
ViChar options[]  = "Simulate=true, DriverSetup= Model=U5303A";

int
main()
{
    std::cout << std::endl;
    
    bool success( false ), simulated( false );

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
            method.mode_ = 0; // digitizer
            method.method_.front_end_range = 1.0;  // V
            method.method_.front_end_offset = 0.0; // V
            method.method_.ext_trigger_level = 1.0;
            method.method_.samp_rate = 3.2e9;
            method.method_.nbr_records = 1;        // MultiRecords
            method.method_.nbr_of_averages = 0;    // digitizer
            method.method_.delay_to_first_sample_ = method.method_.digitizer_delay_to_first_sample = 4.0e-6; // 4us
            method.method_.digitizer_nbr_of_s_to_acquire = method.method_.nbr_of_s_to_acquire_ = 320000; // 100us
            
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
            
            md2->setTriggerLevel( "External1", 1.0 ); // 1V
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

            // digitizer dependent 
            md2->setAcquisitionNumRecordsToAcquire( method.method_.nbr_records );
            md2->setAcquisitionMode( AGMD2_VAL_ACQUISITION_MODE_NORMAL );
            // <--

            md2->CalibrationSelfCalibrate();

            std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();

            std::shared_ptr< acqrscontrols::u5303a::waveform > w1;
            
            std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > > vec;

            const size_t replicates = 1000;

            for ( int i = 0; i < replicates; ++i ) {

                md2->AcquisitionInitiate();
                md2->AcquisitionWaitForAcquisitionComplete( 1000 );

                u5303a::digitizer::readData( *md2, method, vec );
                if ( i == 0 && !vec.empty() )
                    w1 = vec[0];
            }
            
            uint64_t ns = std::chrono::duration_cast< std::chrono::nanoseconds >( std::chrono::steady_clock::now() - tp ).count();
            double s = double(ns) * 1.0e-9;
            std::cout << "Took " << s << " seconds; " << double(replicates) / s << "Hz" << std::endl;
        }
        
    }
    
    return 0;
}
