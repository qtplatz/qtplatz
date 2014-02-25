/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "digitizer.hpp"
#include <adlog/logger.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/logic/tribool.hpp>
#include <mutex>
#include <thread>

#import "IviDriverTypeLib.dll" no_namespace
#import "AgMD2.dll" no_namespace

namespace u5303a { namespace detail {

        class task {
            task();
            ~task();
        public:
            static task * instance();

            inline boost::asio::io_service& io_service() { return io_service_; }
            
            void terminate();
            bool initialize();

        private:
            static task * instance_;
            static std::mutex mutex_;

            IAgMD2Ptr spAgDrvr_;
            std::vector< std::thread > threads_;
            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;

            bool handle_initial_setup( int nDelay, int nSamples, int nAverage );
            bool handle_terminating();
            bool handle_acquire();
            boost::tribool acquire();
        };

    }
}

using namespace u5303a;
using namespace u5303a::detail;

task * task::instance_ = 0;
std::mutex task::mutex_;

digitizer::digitizer()
{
}

digitizer::~digitizer()
{
}

bool
digitizer::peripheral_terminate()
{
    task::instance()->terminate();
    return true;
}

bool
digitizer::peripheral_initialize()
{
    return task::instance()->initialize();
}

////////////////////

task::task() : work_( io_service_ )
{
    threads_.push_back( std::thread( boost::bind( &boost::asio::io_service::run, &io_service_ ) ) );
    io_service_.post( [&] { ::CoInitialize( 0 ); } );
	io_service_.post( [&] { spAgDrvr_.CreateInstance(__uuidof(AgMD2)); } );
}

task::~task()
{
}

task *
task::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new task();
    }
    return instance_;
}

bool
task::initialize()
{
    ADTRACE() << "u5303a digitizer initializing...";
    io_service_.post( [&] { handle_initial_setup( 32, 1024 * 10, 2 ); } );        
    return true;
}

void
task::terminate()
{
    io_service_.post( [&] { ::CoUninitialize(); } );
    io_service_.stop();
    for ( std::thread& t: threads_ )
        t.join();
    threads_.clear();
}

///////////////////////////////////////
         //// Parameters ////
//////////////////////////////////////
double front_end_range = 2.0;    // 1V,2V range
double front_end_offset = 0.0;   // [-0.5V,0.5V], [-1V,1V] offset
double ext_trigger_level = 0.0;  // external trigger threshold
//
long nbr_of_s_to_acquire = 100000; // from 1 to 480,000 samples
long nbr_of_averages = 19999;      // number of averages minus one. >From 0 to 519,999 averages in steps of 8. For instance 0,7,15E
long delay_to_first_s = 0;       // from 0 to 16,000,000 "blocks". Each block shifts by 10ns. 
 
long invert_signal = 0;          //0-> no inversion , 1-> signal inverted
long nsa = 0x0;                  //bit[31]->enable, bits[11:0]->threshold
//////////////////////////////////////

bool
task::handle_initial_setup( int nDelay, int nSamples, int nAverage )
{
	BSTR strResourceDesc = L"PXI3::0::0::INSTR";
    
    // If desired, use 'DriverSetup= CAL=0' to prevent digitizer from doing a SelfCal (~1 seconds) each time
    // it is initialized or reset which is the default behavior. By default set to false.
    // CUSTOM FIRMWARE NAME: U5303ADPULX2AVE.bit
	//BSTR strInitOptions = L"Simulate=false, DriverSetup=UserDpuA=U5303ADPULX2AVE.bit, Trace=false";
	BSTR strInitOptions = L"Simulate=false, DriverSetup=UserDpuA=U5303ADPULX2DIG.bit, Trace=false";
    // note that CAL=0 is not necessary with MD2 as the card is initialized without calibration.
				
    VARIANT_BOOL idQuery = VARIANT_TRUE;
    VARIANT_BOOL reset   = VARIANT_TRUE;

    bool success = false;
    try {
        success = spAgDrvr_->Initialize( strResourceDesc, idQuery, reset, strInitOptions ) == S_OK;
    } catch ( _com_error & e ) {
        ADERROR() << e.Description() << ", " << e.ErrorMessage();
    }
    if ( !success ) {
        try {
            BSTR strInitOptions = L"Simulate=true, DriverSetup= Model=U5303A, Trace=true";
            success = spAgDrvr_->Initialize( strResourceDesc, idQuery, reset, strInitOptions ) == S_OK;
        } catch ( _com_error & e ) {
            ADERROR() << e.Description() << ", " << e.ErrorMessage();
        }
    }
    if ( success ) {
        ADTRACE() << "Identifier:  " << std::wstring(spAgDrvr_->Identity->Identifier.GetBSTR());
        ADTRACE() << "Revision:    " << std::wstring(spAgDrvr_->Identity->Revision.GetBSTR());
        ADTRACE() << "Vendor:      " << std::wstring(spAgDrvr_->Identity->Vendor.GetBSTR());
        ADTRACE() << "Description: " << std::wstring(spAgDrvr_->Identity->Description.GetBSTR());
        ADTRACE() << "Model:       " << std::wstring(spAgDrvr_->Identity->InstrumentModel.GetBSTR());
        ADTRACE() << "FirmwareRev: " << std::wstring(spAgDrvr_->Identity->InstrumentFirmwareRevision.GetBSTR());

        IAgMD2LogicDevicePtr spDpuA = spAgDrvr_->LogicDevices->Item[L"DpuA"];	
        IAgMD2LogicDeviceMemoryBankPtr spDDR3A = spDpuA->MemoryBanks->Item[L"DDR3A"];
        IAgMD2LogicDeviceMemoryBankPtr spDDR3B = spDpuA->MemoryBanks->Item[L"DDR3B"];
		
        // Create smart pointers to Channels and TriggerSource interfaces
        IAgMD2ChannelPtr spCh1 = spAgDrvr_->Channels->Item[L"Channel1"];
        IAgMD2TriggerSourcePtr spTrigSrc = spAgDrvr_->Trigger->Sources->Item[L"External1"];
        
        spCh1->TimeInterleavedChannelList = "Channel2"; 
        spCh1->PutRange(front_end_range);              
        spCh1->PutOffset(front_end_offset);           	
        
        // Setup triggering
        spAgDrvr_->Trigger->ActiveSource = "External1"; 
        spTrigSrc->PutLevel(ext_trigger_level);      
		
        // Calibrate
        ADTRACE() << "Calibrating...";
        spAgDrvr_->Calibration->SelfCalibrate(); 
		
        // Set the mode FDK
        ADTRACE() << "Mode FDK...";
        spAgDrvr_->Acquisition->Mode = AgMD2AcquisitionModeUserFDK;

        // Set the sample rate and nbr of samples to acquire
        double sample_rate = 3.2E9;
        spAgDrvr_->Acquisition->PutSampleRate(sample_rate);                           
        spAgDrvr_->Acquisition->UserControl->PostTrigger = (nbr_of_s_to_acquire/32)+2; 
        spAgDrvr_->Acquisition->UserControl->PreTrigger = 0; 
			
        // Start on trigger
        spAgDrvr_->Acquisition->UserControl->StartOnTriggerEnabled = 1;

        // Full bandwidth
        spCh1->Filter->Bypass = 1;

        // Apply the Settings 
        printf("Apply setup...\n");
        spAgDrvr_->Acquisition->ApplySetup();
			
        long seg_depth = (nbr_of_s_to_acquire >> 5) + 5;
        long seg_ctrl = 0x80000;   // Averager mode, Analog trigger

        if (invert_signal == 1) 	{
            seg_ctrl=seg_ctrl+0x400000;
        }
        long delay_next_acq = 2;

        spDpuA->WriteRegisterInt32(0x3300, seg_depth);        
        spDpuA->WriteRegisterInt32(0x3304, nbr_of_averages);   
        spDpuA->WriteRegisterInt32(0x3308, seg_ctrl);        
        spDpuA->WriteRegisterInt32(0x3318, nbr_of_averages);  
        spDpuA->WriteRegisterInt32(0x331c, nsa);              
        spDpuA->WriteRegisterInt32(0x3320, delay_to_first_s); 
        spDpuA->WriteRegisterInt32(0x3324, delay_next_acq);   
		
        Sleep(1000);

        // Memory settings
        spDDR3A->PutAccessControl(AgMD2LogicDeviceMemoryBankAccessControlUserFirmware);
        spDDR3B->PutAccessControl(AgMD2LogicDeviceMemoryBankAccessControlUserFirmware);
			
        Sleep(1000);        
        //Start the acquisition
        spAgDrvr_->Acquisition->UserControl->StartSegmentation();                              	
        spAgDrvr_->Acquisition->UserControl->StartProcessing(AgMD2UserControlProcessingType1); 	
		
			//Wait for the end of the acquisition
        long wait_for_end = 0x80000000;  	
        Sleep(200);
        while (wait_for_end>=0x80000000) {
            spDpuA->ReadRegisterInt32(0x3308,&wait_for_end);	
            Sleep(200);
        }
        
        //Read data
        ADTRACE() << "Reading data...";
        SAFEARRAY* psaWfmDataRaw = NULL;  // Driver will allocate to required size --> To store data samples
        long words_32bits = nbr_of_s_to_acquire;
        __int64 ActualElements, FirstValidElement;
        spDpuA->ReadIndirectInt32(0x11,0,words_32bits,&psaWfmDataRaw,&ActualElements,&FirstValidElement); 
        
        //Store acquired data in a file
        ofstream myfile_raw;
        myfile_raw.open ("Averager_Output.txt");
		
        //Memory will be freed when the smart data type goes out of scope
        void *pVoid = 0;
        SafeArrayAccessData(psaWfmDataRaw, &pVoid);
        const long *pLongs = reinterpret_cast<long *>(pVoid);
		
        // Output the waveform data using FirstValidPoint and ActualPoints as array may be larger than actual data.
        printf("Waveform Points: %d\n\n", ActualElements);
        for (long i = (long)FirstValidElement; i < ((long)FirstValidElement +(long)ActualElements); i++) {
            _int32 s1 = pLongs[i];
            // save the data to file in raw data and volts 
            myfile_raw << s1 << ";" << s1 / nbr_of_averages * (front_end_range / 8192) - front_end_range / 2 << endl;
        }
        SafeArrayUnaccessData(psaWfmDataRaw);   
    }
	return success;
}

bool
task::handle_terminating()
{
	return false;
}

bool
task::handle_acquire()
{
    // scedule for next acquire
    io_service_.post( [&] { handle_acquire(); } );
#if 0
    if ( acquire() != false ) {
        if ( waitForEndOfAcquisition( 3000 ) == acqDone ) {
            auto avgr = std::make_shared< infitofinterface::AveragerData >();

            if ( readData( *avgr ) ) {
                if ( software_events_ ) {
                    avgr->wellKnownEvents |= software_events_; // marge with hardware events
                    software_events_ = 0;

                    // set time for injection to zero (a.k.a. retention time)
                    if ( avgr->wellKnownEvents & SignalObserver::wkEvent_INJECT ) {
                        averager_inject_usec_ = avgr->uptime;
                        avgr->timeSinceInject = 0;
                    }
                }

                assert( avgr->nbrSamples );

                TOFTask::instance()->putq( avgr );
            }
        } else {
            adportable::debug(__FILE__, __LINE__)
                << "===== handle_acquire waitForEndOfAcquisitioon == not handled.";
        }
        return true;
    }
#endif
    return false;
}

boost::tribool
task::acquire()
{
#if 0
    ViStatus st = AcqrsD1_acquire( inst_ );
    if ( st == VI_SUCCESS )
        return true;

    if ( st != VI_SUCCESS ) {
        Logger( error_msg( st, L"Acqiris::acquire" ), EventLog::pri_ERROR );

        if ( st == ACQIRIS_ERROR_INSTRUMENT_RUNNING ) {
            ViStatus status = AcqrsD1_stopAcquisition(inst_);
            Logger( error_msg( status, L"Acqiris::acquire - stopAcquisition" ), EventLog::pri_ERROR );
            return boost::indeterminate;
        }
    }
#endif
    return false;
}

