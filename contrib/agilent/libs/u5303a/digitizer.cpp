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
#include <adportable/string.hpp>
#include "safearray.hpp"
#include <adlog/logger.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/type_traits.hpp>
#include <boost/variant.hpp>
#include <mutex>
#include <thread>
#include <algorithm>
#include <chrono>

#import "IviDriverTypeLib.dll" no_namespace
#import "AgMD2.dll" no_namespace

namespace u5303a { namespace detail {

        enum DeviceType { Simulate, UserFDK };
        double front_end_range = 2.0;    // 1V,2V range
        double front_end_offset = 0.0;   // [-0.5V,0.5V], [-1V,1V] offset
        double ext_trigger_level = 0.0;  // external trigger threshold
        //
        long nbr_of_s_to_acquire = 100000; // from 1 to 480,000 samples
        long nbr_of_averages = 19999;      // number of averages minus one. >From 0 to 519,999 averages in steps of 8. For instance 0,7,15
        long delay_to_first_s = 0;       // from 0 to 16,000,000 "blocks". Each block shifts by 10ns. 
        long invert_signal = 0;          //0-> no inversion , 1-> signal inverted
        long nsa = 0x0;                  //bit[31]->enable, bits[11:0]->threshold

        template<DeviceType> struct device {
            static bool initial_setup( task& );
            static boost::tribool acquire( task& );
            static bool waitForEndOfAcquisition( task&, int timeout );
        };

        class task {
            task();
            ~task();
        public:
            static task * instance();

            inline boost::asio::io_service& io_service() { return io_service_; }
            
            void terminate();
            bool initialize();
            bool prepare_for_run( const method& );
            bool run();
            bool stop();
            bool trigger_inject_out();

            void connect( digitizer::command_reply_type f );
            void disconnect( digitizer::command_reply_type f );
            void connect( digitizer::waveform_reply_type f );
            void disconnect( digitizer::waveform_reply_type f );

        private:
            template<DeviceType> friend struct device;
            static task * instance_;
            static std::mutex mutex_;

            IAgMD2Ptr spAgDrvr_;
            std::vector< std::thread > threads_;
            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;
            bool simulated_;

            std::vector< digitizer::command_reply_type > reply_handlers_;
            std::vector< digitizer::waveform_reply_type > waveform_handlers_;

            bool handle_initial_setup( int nDelay, int nSamples, int nAverage );
            bool handle_terminating();
            bool handle_acquire();
            bool handle_prepare_for_run( const method& );
            boost::tribool acquire();
            bool waitForEndOfAcquisition( int timeout );
            void error_reply( const _com_error& e, const std::string& );
            bool readData( waveform& );
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
digitizer::peripheral_prepare_for_run( const method& m )
{
    return task::instance()->prepare_for_run( m );
}

bool
digitizer::peripheral_initialize()
{
    return task::instance()->initialize();
}

bool
digitizer::peripheral_run()
{
    return task::instance()->run();
}

bool
digitizer::peripheral_stop()
{
    return task::instance()->stop();
}

bool
digitizer::peripheral_trigger_inject()
{
    return task::instance()->trigger_inject_out();
}

void
digitizer::connect_reply( command_reply_type f )
{
    task::instance()->connect( f );
}

void
digitizer::disconnect_reply( command_reply_type f )
{
    task::instance()->disconnect( f );
}

void
digitizer::connect_waveform( waveform_reply_type f )
{
    task::instance()->connect( f );
}

void
digitizer::disconnect_waveform( waveform_reply_type f )
{
    task::instance()->disconnect( f );
}

////////////////////

task::task() : work_( io_service_ )
             , simulated_( false )
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

bool
task::prepare_for_run( const method& m )
{
    ADTRACE() << "u5303a digitizer prepare for run...";
    io_service_.post( [&] { handle_prepare_for_run(m); } );        
    io_service_.post( [&] { handle_acquire(); } );        
    return true;
}

bool
task::run()
{
    return true;
}

bool
task::stop()
{
    return true;
}

bool
task::trigger_inject_out()
{
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
//////////////////////////////////////

bool
task::handle_initial_setup( int nDelay, int nSamples, int nAverage )
{
	BSTR strResourceDesc = L"PXI3::0::0::INSTR";
    
    // If desired, use 'DriverSetup= CAL=0' to prevent digitizer from doing a SelfCal (~1 seconds) each time
    // it is initialized or reset which is the default behavior. By default set to false.
    // CUSTOM FIRMWARE NAME: U5303ADPULX2AVE.bit

	//BSTR strInitOptions = L"Simulate=false, DriverSetup=UserDpuA=U5303ADPULX2AVE.bit, Trace=false"; // <-- this file does not exist
	BSTR strInitOptions = L"Simulate=false, DriverSetup=UserDpuA=U5303ADPULX2DIG.bit, Trace=false";
    // note that CAL=0 is not necessary with MD2 as the card is initialized without calibration.
				
    VARIANT_BOOL idQuery = VARIANT_TRUE;
    VARIANT_BOOL reset   = VARIANT_TRUE;

    simulated_ = false;
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
            simulated_ = true;
        } catch ( _com_error & e ) {
            ADERROR() << e.Description() << ", " << e.ErrorMessage();
        }
    }

    if ( success ) {
		identify ident;

        ident.Identifier       = static_cast< const char *>( _bstr_t( spAgDrvr_->Identity->Identifier.GetBSTR() ) );
        ident.Revision         = static_cast< const char *>( _bstr_t( spAgDrvr_->Identity->Revision.GetBSTR() ) );
        ident.Vendor           = static_cast< const char *>( _bstr_t( spAgDrvr_->Identity->Vendor.GetBSTR() ) );
        ident.Description      = static_cast< const char *>( _bstr_t( spAgDrvr_->Identity->Description.GetBSTR() ) );
        ident.InstrumentModel  = static_cast< const char *>( _bstr_t( spAgDrvr_->Identity->InstrumentModel.GetBSTR() ) );
        ident.FirmwareRevision = static_cast< const char *>( _bstr_t( spAgDrvr_->Identity->InstrumentFirmwareRevision.GetBSTR() ) );

        for ( auto& reply: reply_handlers_ ) reply( "Identifier", ident.Identifier );
        for ( auto& reply: reply_handlers_ ) reply( "Revision", ident.Revision );
        for ( auto& reply: reply_handlers_ ) reply( "Description", ident.Description );
        for ( auto& reply: reply_handlers_ ) reply( "InstrumentModel", ident.InstrumentModel );
        for ( auto& reply: reply_handlers_ ) reply( "InstrumentFirmwareRevision", ident.FirmwareRevision );

        if ( simulated_ )
            device<Simulate>::initial_setup( *this );
        else
            device<UserFDK>::initial_setup( *this );
    }
    for ( auto& reply: reply_handlers_ )
        reply( "InitialSetup", ( success ? "success" : "failed" ) );
	return success;
}

bool
task::handle_terminating()
{
	return false;
}

bool
task::handle_prepare_for_run( const method& )
{
	return true;
}

bool
task::handle_acquire()
{
    io_service_.post( [&] { handle_acquire(); } );    // scedule for next acquire

    typedef device<UserFDK> inst;

    if ( acquire() != false ) {
        if ( waitForEndOfAcquisition( 3000 ) ) {
            auto avgr = std::make_shared< waveform >();
            if ( readData( *avgr ) ) {
                // if ( software_events_ ) {
                //     avgr->wellKnownEvents |= software_events_; // marge with hardware events
                //     software_events_ = 0;

                //     // set time for injection to zero (a.k.a. retention time)
                //     if ( avgr->wellKnownEvents & SignalObserver::wkEvent_INJECT ) {
                //         averager_inject_usec_ = avgr->uptime;
                //         avgr->timeSinceInject = 0;
                //     }
                // }
                // assert( avgr->nbrSamples );
                for ( auto& reply: waveform_handlers_ )
                    reply( avgr.get() );
            }
        } else {
            ADTRACE() << "===== handle_acquire waitForEndOfAcquisitioon == not handled.";
        }
        return true;
    }
    return false;
}

boost::tribool
task::acquire()
{
    if ( simulated_ )
        return device<Simulate>::acquire( *this );
    else
        return device<UserFDK>::acquire( *this );
}

bool
task::waitForEndOfAcquisition( int timeout )
{
    if ( simulated_ )
        return device<Simulate>::waitForEndOfAcquisition( *this, timeout );
    else
        return device<UserFDK>::waitForEndOfAcquisition( *this, timeout );
}

bool
task::readData( waveform& data )
{
    IAgMD2LogicDevicePtr spDpuA = spAgDrvr_->LogicDevices->Item[L"DpuA"];

    long words_32bits = nbr_of_s_to_acquire;

    try {
		SAFEARRAY * psaWfmDataRaw(0);
        spDpuA->ReadIndirectInt32(0x11, 0, words_32bits, &psaWfmDataRaw, &data.actualElements_, &data.firstValidElement_);
		safearray_t<int32_t> sa( psaWfmDataRaw );
        size_t size = sa.size();
        data.d_.resize( size );
		std::copy( sa.data(), sa.data() + size, data.d_.begin() );
    } catch ( _com_error& e ) {
        ADERROR() << "digitier::task::readData: " << e.Description() << e.ErrorMessage();
    }
    return true;
}

void
task::connect( digitizer::command_reply_type f )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    reply_handlers_.push_back( f );
}

void
task::disconnect( digitizer::command_reply_type f )
{
    std::lock_guard< std::mutex > lock( mutex_ );    
	auto it = std::remove_if( reply_handlers_.begin(), reply_handlers_.end(), [=]( const digitizer::command_reply_type& t ){
            return t == f;
        });
    reply_handlers_.erase( it, reply_handlers_.end() );
}

void
task::connect( digitizer::waveform_reply_type f )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    waveform_handlers_.push_back( f );
}

void
task::disconnect( digitizer::waveform_reply_type f )
{
    std::lock_guard< std::mutex > lock( mutex_ );    
	auto it = std::remove_if( waveform_handlers_.begin(), waveform_handlers_.end(), [=]( const digitizer::waveform_reply_type& t ){
            return t == f;
        });
    waveform_handlers_.erase( it, waveform_handlers_.end() );
}

void
task::error_reply( const _com_error& e, const std::string& method )
{
    _bstr_t msg( _bstr_t(L"Error: ") + e.Description() + L": " + e.ErrorMessage() );
    for ( auto& reply: reply_handlers_ )
        reply( method, static_cast< const char *>( msg ) );
}

///
identify::identify()
{
}

identify::identify( const identify& t ) : Identifier( t.Identifier )
                                        , Revision( t.Revision )
                                        , Vendor( t.Vendor )
                                        , Description( t.Description )
                                        , InstrumentModel( t.InstrumentModel )
                                        , FirmwareRevision( t.FirmwareRevision )
{
}

template<> bool
device<UserFDK>::initial_setup( task& task )
{
    IAgMD2LogicDevicePtr spDpuA = task.spAgDrvr_->LogicDevices->Item[L"DpuA"];	
    IAgMD2LogicDeviceMemoryBankPtr spDDR3A = spDpuA->MemoryBanks->Item[L"DDR3A"];
    IAgMD2LogicDeviceMemoryBankPtr spDDR3B = spDpuA->MemoryBanks->Item[L"DDR3B"];
	
    // Create smart pointers to Channels and TriggerSource interfaces
    IAgMD2ChannelPtr spCh1 = task.spAgDrvr_->Channels->Item[L"Channel1"];
    IAgMD2TriggerSourcePtr spTrigSrc = task.spAgDrvr_->Trigger->Sources->Item[L"External1"];
    
    try {
        spCh1->TimeInterleavedChannelList = "Channel2"; 
        spCh1->PutRange(front_end_range);              
        spCh1->PutOffset(front_end_offset);           	
        
        // Setup triggering
        task.spAgDrvr_->Trigger->ActiveSource = "External1"; 
        spTrigSrc->PutLevel(ext_trigger_level);      
        
        // Calibrate
        ADTRACE() << "Calibrating...";
        task.spAgDrvr_->Calibration->SelfCalibrate();
    } catch ( _com_error& e ) {
        ADERROR() << e.Description() << e.ErrorMessage();
        task.error_reply( e, "..." );
    } 

    try {
        ADTRACE() << "Set the Mode FDK...";
        task.spAgDrvr_->Acquisition->Mode = AgMD2AcquisitionModeUserFDK;
    } catch ( _com_error& e ) {
        ADERROR() << e.Description() << e.ErrorMessage();
        task.error_reply( e, "Mode FDK" );
    }

    // Set the sample rate and nbr of samples to acquire
    double sample_rate = 3.2E9;
    task.spAgDrvr_->Acquisition->PutSampleRate(sample_rate); 
    try {
        task.spAgDrvr_->Acquisition->UserControl->PostTrigger = (nbr_of_s_to_acquire/32) + 2;
    } catch ( _com_error& e ) {
        ADERROR() << e.Description() << e.ErrorMessage();
        task.error_reply( e, "SampleRate" );
    }
    try {
        task.spAgDrvr_->Acquisition->UserControl->PreTrigger = 0; 
    } catch ( _com_error& e ) {
        ADERROR() << e.Description() << e.ErrorMessage();
        task.error_reply( e, "Trigger" );
    }
    try {
        // Start on trigger
        task.spAgDrvr_->Acquisition->UserControl->StartOnTriggerEnabled = 1;
    } catch ( _com_error& e ) {
        ADERROR() << e.Description() << e.ErrorMessage();
        task.error_reply( e, "StartOnTrigger" );
    }
    try {
        // Full bandwidth
        spCh1->Filter->Bypass = 1;
    } catch ( _com_error& e ) {
        ADERROR() << e.Description() << e.ErrorMessage();
        task.error_reply( e, "Bandwidth" );
    }
    try {
        ADTRACE() << "Apply setup...";
        task.spAgDrvr_->Acquisition->ApplySetup();
    } catch ( _com_error& e ) {
        ADERROR() << e.Description() << e.ErrorMessage();
        task.error_reply( e, "ApplySetup" );
    }
    long seg_depth = (nbr_of_s_to_acquire >> 5) + 5;
    long seg_ctrl = 0x80000;   // Averager mode, Analog trigger

    if (invert_signal == 1) 	{
        seg_ctrl=seg_ctrl+0x400000;
    }
    long delay_next_acq = 2;
    try {
        spDpuA->WriteRegisterInt32(0x3300, seg_depth);        
        spDpuA->WriteRegisterInt32(0x3304, nbr_of_averages);   
        spDpuA->WriteRegisterInt32(0x3308, seg_ctrl);        
        spDpuA->WriteRegisterInt32(0x3318, nbr_of_averages);  
        spDpuA->WriteRegisterInt32(0x331c, nsa);              
        spDpuA->WriteRegisterInt32(0x3320, delay_to_first_s); 
        spDpuA->WriteRegisterInt32(0x3324, delay_next_acq);   
    } catch ( _com_error& e ) {
        ADERROR() << e.Description() << e.ErrorMessage();
        task.error_reply( e, "WriteRegisterInt32" );
    }
    std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );

    // Memory settings
    try {
        spDDR3A->PutAccessControl(AgMD2LogicDeviceMemoryBankAccessControlUserFirmware);
        spDDR3B->PutAccessControl(AgMD2LogicDeviceMemoryBankAccessControlUserFirmware);
    } catch ( _com_error& e ) {
        ADERROR() << e.Description() << e.ErrorMessage();
        task.error_reply( e, "AccessControl" );
    }

    std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );

	return true;
}

template<> boost::tribool
device<UserFDK>::acquire( task& task )
{
    //Start the acquisition
    try {
        task.spAgDrvr_->Acquisition->UserControl->StartSegmentation();                              	
        task.spAgDrvr_->Acquisition->UserControl->StartProcessing(AgMD2UserControlProcessingType1); 	
    } catch ( _com_error& e ) {
        ADERROR() << e.Description() << e.ErrorMessage();
        task.error_reply( e, "StartSegmentation" );
    }
    return true;
}

template<> bool
device<UserFDK>::waitForEndOfAcquisition( task& task, int timeout )
{
    //Wait for the end of the acquisition

    long wait_for_end = 0x80000000;
    IAgMD2LogicDevicePtr spDpuA = task.spAgDrvr_->LogicDevices->Item[L"DpuA"];	

    while ( wait_for_end >= 0x80000000 ) {

        try {
            std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
            spDpuA->ReadRegisterInt32( 0x3308, &wait_for_end );

        } catch ( _com_error& e ) {
            ADERROR() << e.Description() << e.ErrorMessage();
            task.error_reply( e, "ReadRegisterInt32" );
            return false;
        }
    }
    return true;
}

template<> bool
device<Simulate>::initial_setup( task& task )
{
    IAgMD2LogicDevicePtr spDpuA = task.spAgDrvr_->LogicDevices->Item[L"DpuA"];	
    IAgMD2LogicDeviceMemoryBankPtr spDDR3A = spDpuA->MemoryBanks->Item[L"DDR3A"];
    IAgMD2LogicDeviceMemoryBankPtr spDDR3B = spDpuA->MemoryBanks->Item[L"DDR3B"];
	
    // Create smart pointers to Channels and TriggerSource interfaces
    IAgMD2ChannelPtr spCh1 = task.spAgDrvr_->Channels->Item[L"Channel1"];
    IAgMD2TriggerSourcePtr spTrigSrc = task.spAgDrvr_->Trigger->Sources->Item[L"External1"];
    
    try {
        spCh1->TimeInterleavedChannelList = "Channel2"; 
        spCh1->PutRange(front_end_range);              
        spCh1->PutOffset(front_end_offset);           	
        
        // Setup triggering
        task.spAgDrvr_->Trigger->ActiveSource = "External1"; 
        spTrigSrc->PutLevel(ext_trigger_level);      
        
        // Calibrate
        ADTRACE() << "Calibrating...";
        task.spAgDrvr_->Calibration->SelfCalibrate();
    } catch ( _com_error& e ) {
        ADERROR() << e.Description() << e.ErrorMessage();
        task.error_reply( e, "..." );
    } 

    try {
        task.spAgDrvr_->Acquisition->Mode = AgMD2AcquisitionModeUserFDK; // error
    } catch ( _com_error& e ) {
        task.error_reply( e, "Mode FDK" );
    }

    // Set the sample rate and nbr of samples to acquire
    double sample_rate = 3.2E9;
    task.spAgDrvr_->Acquisition->PutSampleRate(sample_rate); 

    try {
        // Full bandwidth
        spCh1->Filter->Bypass = 1; // invalid value
    } catch ( _com_error& e ) {
        ADERROR() << e.Description() << e.ErrorMessage();
        task.error_reply( e, "Bandwidth" );
    }

    try {
        task.spAgDrvr_->Acquisition->ApplySetup();
    } catch ( _com_error& e ) {
        ADERROR() << e.Description() << e.ErrorMessage();
    }
    long seg_depth = (nbr_of_s_to_acquire >> 5) + 5;
    long seg_ctrl = 0x80000;   // Averager mode, Analog trigger

    if (invert_signal == 1) 	{
        seg_ctrl=seg_ctrl+0x400000;
    }
    long delay_next_acq = 2;
    std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );

	return true;
}

template<> boost::tribool
device<Simulate>::acquire( task& task )
{
    std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
    return true;
}

template<> bool
device<Simulate>::waitForEndOfAcquisition( task& task, int timeout )
{
    return true;
}
