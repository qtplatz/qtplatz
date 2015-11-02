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

#include "ppio.hpp"
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
            method.method_.TSR_enabled = true;
            
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

            md2->setAcquisitionNumRecordsToAcquire( method.method_.nbr_records );
            md2->setAcquisitionMode( AGMD2_VAL_ACQUISITION_MODE_NORMAL ); // Digitizer mode

            md2->setTSREnabled( method.method_.TSR_enabled );            

            md2->CalibrationSelfCalibrate();
            
            std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();

            std::shared_ptr< acqrscontrols::u5303a::waveform > w1;
            
            std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > > vec;

            contexpr size_t replicates = 100000;

            if ( md2->TSREnabled() ) {

                std::cout << "TSR " << md2->TSREnabled() << std::endl;
                
                

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
