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
#include "simulator.hpp"
#include <adportable/string.hpp>
#include "safearray.hpp"
#include <adlog/logger.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/type_traits.hpp>
#include <boost/variant.hpp>
#include <mutex>
#include <thread>
#include <algorithm>
#include <chrono>

#import "IviDriverTypeLib.dll" no_namespace
#import "AgMD2.dll" no_namespace

#ifdef ERR
# undef ERR
#endif
#ifdef TERR
# undef TERR
#endif

#define ERR(e,m) do { adlog::logger(__FILE__,__LINE__,adlog::LOG_ERROR)<<e.Description()<<", "<<e.ErrorMessage(); error_reply(e,m); } while(0)
#define TERR(e,m) do { adlog::logger(__FILE__,__LINE__,adlog::LOG_ERROR)<<e.Description()<<", "<<e.ErrorMessage(); task.error_reply(e,m); } while(0)

namespace u5303a {

    class simulator;

    namespace detail {

        enum DeviceType { Simulate, UserFDK };

        template<DeviceType> struct device {
            static bool initial_setup( task&, const method& );
            static bool setup( task&, const method& );
            static bool acquire( task& );
            static bool waitForEndOfAcquisition( task&, int timeout );
            static bool readData( task&, waveform& );
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

            inline IAgMD2Ptr& spAgDrvr() { return spAgDrvr_; }
            inline simulator * simulator() { return simulator_; }
            inline const method& method() const { return method_; }
            void error_reply( const _com_error& e, const std::string& );

        private:
            static task * instance_;
            static std::mutex mutex_;

            IAgMD2Ptr spAgDrvr_;
            std::vector< std::thread > threads_;
            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;
            boost::asio::io_service::strand strand_;
            bool simulated_;
            u5303a::method method_;
			u5303a::simulator * simulator_;
            uint32_t serialnumber_;

            std::vector< digitizer::command_reply_type > reply_handlers_;
            std::vector< digitizer::waveform_reply_type > waveform_handlers_;

            bool handle_initial_setup( int nDelay, int nSamples, int nAverage );
            bool handle_terminating();
            bool handle_acquire();
            bool handle_prepare_for_run( const u5303a::method& );
            bool acquire();
            bool waitForEndOfAcquisition( int timeout );
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
             , strand_( io_service_ )
             , simulated_( false )
             , simulator_( 0 )
             , serialnumber_( 0 )
{
    threads_.push_back( std::thread( boost::bind( &boost::asio::io_service::run, &io_service_ ) ) );
    io_service_.post( strand_.wrap( [&] { ::CoInitialize( 0 ); } ) );
    io_service_.post( strand_.wrap( [&] { spAgDrvr_.CreateInstance(__uuidof(AgMD2)); } ) );
}

task::~task()
{
    delete simulator_;
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
    io_service_.post( strand_.wrap( [&] { handle_initial_setup( 32, 1024 * 10, 2 ); } ) );
    return true;
}

bool
task::prepare_for_run( const u5303a::method& m )
{
    ADTRACE() << "u5303a digitizer prepare for run...";
    io_service_.post( strand_.wrap( [&] { handle_prepare_for_run(m); } ) );
    io_service_.post( strand_.wrap( [&] { handle_acquire(); } ) );
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
    u5303a::method m;

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
        ERR(e,"Initialize");
    }
    if ( !success ) {
        try {
            BSTR strInitOptions = L"Simulate=true, DriverSetup= Model=U5303A, Trace=true";
            success = spAgDrvr_->Initialize( strResourceDesc, idQuery, reset, strInitOptions ) == S_OK;
            simulated_ = true;
            simulator_ = new u5303a::simulator;
            // threads for waveform generation
            threads_.push_back( std::thread( boost::bind( &boost::asio::io_service::run, &io_service_ ) ) );
            threads_.push_back( std::thread( boost::bind( &boost::asio::io_service::run, &io_service_ ) ) );
            threads_.push_back( std::thread( boost::bind( &boost::asio::io_service::run, &io_service_ ) ) );
        } catch ( _com_error & e ) {
            ERR( e, "Initialize" );
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
            device<Simulate>::initial_setup( *this, m );
        else
            device<UserFDK>::initial_setup( *this, m );
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
task::handle_prepare_for_run( const u5303a::method& m )
{
    if ( simulated_ )
        device<Simulate>::setup( *this, m );
    else
        device<UserFDK>::setup( *this, m );
	return true;
}

bool
task::handle_acquire()
{
    static int counter_;
    ADTRACE() << "handle_acquire : " << counter_++;

    io_service_.post( strand_.wrap( [&] { handle_acquire(); } ) );    // scedule for next acquire

    if ( acquire() ) {
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

bool
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
    data.serialnumber_ = serialnumber_++;
    if ( simulated_ )
        return device<Simulate>::readData( *this, data );
    else
        return device<UserFDK>::readData( *this, data );
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
device<UserFDK>::initial_setup( task& task, const method& m )
{
    IAgMD2LogicDevicePtr spDpuA = task.spAgDrvr()->LogicDevices->Item[L"DpuA"];	
    IAgMD2LogicDeviceMemoryBankPtr spDDR3A = spDpuA->MemoryBanks->Item[L"DDR3A"];
    IAgMD2LogicDeviceMemoryBankPtr spDDR3B = spDpuA->MemoryBanks->Item[L"DDR3B"];
	
    // Create smart pointers to Channels and TriggerSource interfaces
    IAgMD2ChannelPtr spCh1 = task.spAgDrvr()->Channels->Item[L"Channel1"];
    IAgMD2TriggerSourcePtr spTrigSrc = task.spAgDrvr()->Trigger->Sources->Item[L"External1"];
    
    try { spCh1->TimeInterleavedChannelList = "Channel2";       } catch ( _com_error& e ) { TERR(e, "TimeInterleavedChannelList");  }
    try { spCh1->PutRange(m.front_end_range);                   } catch ( _com_error& e ) { TERR(e, "Range");  }
    try { spCh1->PutOffset(m.front_end_offset);                 } catch ( _com_error& e ) { TERR(e, "Offset");  }
    // Setup triggering
    try { task.spAgDrvr()->Trigger->ActiveSource = "External1"; } catch ( _com_error& e ) { TERR(e, "Trigger::ActiveSource");  }
    try { spTrigSrc->PutLevel(m.ext_trigger_level);             } catch ( _com_error& e ) { TERR(e, "TriggerSource::Level");  }
        
    // Calibrate
    ADTRACE() << "Calibrating...";
    try { task.spAgDrvr()->Calibration->SelfCalibrate(); } catch ( _com_error& e ) { TERR(e, "Calibration::SelfCalibrate"); }

    ADTRACE() << "Set the Mode FDK...";
    try { task.spAgDrvr()->Acquisition->Mode = AgMD2AcquisitionModeUserFDK; } catch (_com_error& e) { TERR(e,"Acquisition::Mode"); }

    // Set the sample rate and nbr of samples to acquire
    const double sample_rate = 3.2E9;
    try { task.spAgDrvr()->Acquisition->PutSampleRate(sample_rate); }  catch (_com_error& e) { TERR(e,"SampleRate"); }
    try { task.spAgDrvr()->Acquisition->UserControl->PostTrigger = (m.nbr_of_s_to_acquire/32) + 2; } catch ( _com_error& e ) { TERR(e,"PostTrigger"); }
    try { task.spAgDrvr()->Acquisition->UserControl->PreTrigger = 0; } catch ( _com_error& e ) { TERR(e,"PreTrigger"); }

    // Start on trigger
    try { task.spAgDrvr()->Acquisition->UserControl->StartOnTriggerEnabled = 1; } catch ( _com_error& e ) { TERR( e,"StartOnTrigger" ); }

    // Full bandwidth
    try { spCh1->Filter->Bypass = 1; } catch ( _com_error& e ) { TERR( e, "Bandwidth" ); }

    ADTRACE() << "Apply setup...";
    try { task.spAgDrvr()->Acquisition->ApplySetup();  } catch ( _com_error& e ) { TERR( e, "ApplySetup" ); }

    long seg_depth = (m.nbr_of_s_to_acquire >> 5) + 5;
    long seg_ctrl = 0x80000;   // Averager mode, Analog trigger

    if (m.invert_signal == 1) 	{
        seg_ctrl = seg_ctrl + 0x400000;
    }
    long delay_next_acq = 2;
    try { spDpuA->WriteRegisterInt32(0x3300, seg_depth);         } catch ( _com_error& e ) { TERR(e, "WriteRegisterInt32,0x3300"); }
    try { spDpuA->WriteRegisterInt32(0x3304, m.nbr_of_averages); } catch ( _com_error& e ) { TERR(e, "WriteRegisterInt32,0x3304"); }
    try { spDpuA->WriteRegisterInt32(0x3308, seg_ctrl);          } catch ( _com_error& e ) { TERR(e, "WriteRegisterInt32,0x3308"); }
    try { spDpuA->WriteRegisterInt32(0x3318, m.nbr_of_averages); } catch ( _com_error& e ) { TERR(e, "WriteRegisterInt32,0x3318"); }
    try { spDpuA->WriteRegisterInt32(0x331c, m.nsa);             } catch ( _com_error& e ) { TERR(e, "WriteRegisterInt32,0x331c"); }
    try { spDpuA->WriteRegisterInt32(0x3320, m.delay_to_first_s);} catch ( _com_error& e ) { TERR(e, "WriteRegisterInt32,0x3320"); }
    try { spDpuA->WriteRegisterInt32(0x3324, delay_next_acq);    } catch ( _com_error& e ) { TERR(e, "WriteRegisterInt32,0x3324"); }

    std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );

    // Memory settings
    try { spDDR3A->PutAccessControl(AgMD2LogicDeviceMemoryBankAccessControlUserFirmware); } catch (_com_error& e){TERR(e,"DDR3A::AccessControl");}
    try { spDDR3B->PutAccessControl(AgMD2LogicDeviceMemoryBankAccessControlUserFirmware); } catch (_com_error& e){TERR(e,"DDR3B::AccessControl");}

    std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );

	return true;
}

template<> bool
device<UserFDK>::setup( task& task, const method& m )
{
    return device<UserFDK>::initial_setup( task, m );
}

template<> bool
device<UserFDK>::acquire( task& task )
{
    //Start the acquisition
    try { task.spAgDrvr()->Acquisition->UserControl->StartSegmentation(); } catch ( _com_error& e ) { TERR(e, "StartSegmentation"); }
    try { task.spAgDrvr()->Acquisition->UserControl->StartProcessing(AgMD2UserControlProcessingType1); } catch ( _com_error& e ) {
        TERR( e, "StartProcessing" ); }
    return true;
}

template<> bool
device<UserFDK>::waitForEndOfAcquisition( task& task, int timeout )
{
    //Wait for the end of the acquisition

    long wait_for_end = 0x80000000;
    IAgMD2LogicDevicePtr spDpuA = task.spAgDrvr()->LogicDevices->Item[L"DpuA"];	

    while ( wait_for_end >= 0x80000000 ) {
        std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
        try { spDpuA->ReadRegisterInt32( 0x3308, &wait_for_end ); } catch ( _com_error& e ) { TERR(e, "ReadRegisterInt32"); }
    }
    return true;
}

template<> bool
device<UserFDK>::readData( task& task, waveform& data )
{
    IAgMD2LogicDevicePtr spDpuA = task.spAgDrvr()->LogicDevices->Item[L"DpuA"];

	long words_32bits = task.method().nbr_of_s_to_acquire;

    try {
		SAFEARRAY * psaWfmDataRaw(0);
        spDpuA->ReadIndirectInt32(0x11, 0, words_32bits, &psaWfmDataRaw, &data.actualElements_, &data.firstValidElement_);
		safearray_t<int32_t> sa( psaWfmDataRaw );
        size_t size = sa.size();
        data.d_.resize( size );
		std::copy( sa.data(), sa.data() + size, data.d_.begin() );
    } catch ( _com_error& e ) {
        TERR(e,"readData::ReadIndirectInt32");
    }
    return true;
}


template<> bool
device<Simulate>::initial_setup( task& task, const method& m )
{
    IAgMD2LogicDevicePtr spDpuA = task.spAgDrvr()->LogicDevices->Item[L"DpuA"];	
    IAgMD2LogicDeviceMemoryBankPtr spDDR3A = spDpuA->MemoryBanks->Item[L"DDR3A"];
    IAgMD2LogicDeviceMemoryBankPtr spDDR3B = spDpuA->MemoryBanks->Item[L"DDR3B"];
	
    // Create smart pointers to Channels and TriggerSource interfaces
    IAgMD2ChannelPtr spCh1 = task.spAgDrvr()->Channels->Item[L"Channel1"];
    IAgMD2TriggerSourcePtr spTrigSrc = task.spAgDrvr()->Trigger->Sources->Item[L"External1"];

    try { spCh1->TimeInterleavedChannelList = "Channel2";       } catch ( _com_error& e ) { TERR(e, "TimeInterleavedChannelList");  }
    try { spCh1->PutRange(m.front_end_range);                   } catch ( _com_error& e ) { TERR(e, "Range");  }
    try { spCh1->PutOffset(m.front_end_offset);                 } catch ( _com_error& e ) { TERR(e, "Offset");  }
    // Setup triggering
    try { task.spAgDrvr()->Trigger->ActiveSource = "External1"; } catch ( _com_error& e ) { TERR(e, "Trigger::ActiveSource");  }
    try { spTrigSrc->PutLevel(m.ext_trigger_level);             } catch ( _com_error& e ) { TERR(e, "TriggerSource::Level");  }

    // Calibrate
    ADTRACE() << "Calibrating...";
    try { task.spAgDrvr()->Calibration->SelfCalibrate(); } catch ( _com_error& e ) { TERR(e, "Calibration::SelfCalibrate"); }

    // Set the sample rate and nbr of samples to acquire
    double sample_rate = 3.2E9;
    try { task.spAgDrvr()->Acquisition->PutSampleRate(sample_rate); }  catch (_com_error& e) { TERR(e,"SampleRate"); }

    try { spCh1->Filter->Bypass = 1; } catch ( _com_error& e ) { TERR( e, "Bandwidth" ); } // invalid value

    ADTRACE() << "Apply setup...";
    try { task.spAgDrvr()->Acquisition->ApplySetup();  } catch ( _com_error& e ) { TERR( e, "ApplySetup" ); }

    std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );

	return true;
}

template<> bool
device<Simulate>::setup( task& task, const method& m )
{
    return device<Simulate>::initial_setup( task, m );
}

template<> bool
device<Simulate>::acquire( task& task )
{
    if ( simulator * simulator = task.simulator() )
        return simulator->acquire( task.io_service() );
    return false;
}

template<> bool
device<Simulate>::waitForEndOfAcquisition( task& task, int timeout )
{
    if ( simulator * simulator = task.simulator() )
        return simulator->waitForEndOfAcquisition();
    return false;
}

template<> bool
device<Simulate>::readData( task& task, waveform& data )
{
    if ( simulator * simulator = task.simulator() )
        return simulator->readData( data );
    return false;
}
