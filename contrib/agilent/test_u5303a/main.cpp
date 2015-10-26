
#include <u5303a/agmd2.hpp>
#include <acqrscontrols/u5303a/identify.hpp>
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
        
            auto ident = std::make_shared< acqrscontrols::u5303a::identify >();
            md2->Identify( *ident );

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

            md2->setTriggerDelay( 4.0e-6 );  // 4us
            
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
            std::cout << "SampleRate: " << md2->SampleRate() << std::endl;            

            md2->setAcquisitionRecordSize( 320000 );  // 100us @ 3.2GS/s

            // digitizer dependent 
            md2->setAcquisitionNumRecordsToAcquire( 1 );
            md2->setAcquisitionMode( AGMD2_VAL_ACQUISITION_MODE_NORMAL );
            // <--

            md2->CalibrationSelfCalibrate();

            std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();

            for ( ;; ) {
                md2->AcquisitionInitiate();
                md2->AcquisitionWaitForAcquisitionComplete( 1000 );

                std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > > vec;
                
                
            }
        }
        
    }
    
    return 0;
}
