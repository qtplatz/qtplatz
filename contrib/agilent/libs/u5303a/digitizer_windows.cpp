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
#include "sampleprocessor.hpp"
#include "safearray.hpp"
#include <acqrscontrols/u5303a/method.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/serializer.hpp>
#include <adportable/string.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <workaround/boost/asio.hpp>
#include <boost/type_traits.hpp>
#include <boost/variant.hpp>
#include <boost/bind.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>

#include <mutex>
#include <thread>
#include <algorithm>
#include <chrono>
#include <atomic>

#import "GlobMgr.dll"          no_namespace // VISA-COM I/O functionality
#import "IviDriverTypeLib.dll" no_namespace
#import "AgMD2.dll" no_namespace

#ifdef ERR
# undef ERR
#endif
#ifdef TERR
# undef TERR
#endif

#define DIGITIZER_MODE_TEST 0

#define ERR(e,m) do { adlog::logger(__FILE__,__LINE__,adlog::LOG_WARN)<<e.Description()<<", "<<e.ErrorMessage(); error_reply(e,m); } while(0)
#define TERR(e,m) do { adlog::logger(__FILE__,__LINE__,adlog::LOG_WARN)<<e.Description()<<", "<<e.ErrorMessage(); task.error_reply(e,m); } while(0)

namespace u5303a {

    namespace detail {

        enum DeviceType { Simulate, Averager, Digitizer };

        template<DeviceType> struct device {
            static bool initial_setup( task&, const acqrscontrols::u5303a::method&, const std::string& options );
            static bool setup( task&, const acqrscontrols::u5303a::method& );
            static bool acquire( task& );
            static bool waitForEndOfAcquisition( task&, int timeout );
            static bool readData( task&, acqrscontrols::u5303a::waveform& );
            static bool readData( task&, uint64_t numRecords, std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > >& );
        };

        class task {
            task();
            ~task();
        public:
            static task * instance();
            static const std::chrono::steady_clock::time_point uptime_;
            static const uint64_t tp0_;
            std::exception_ptr exptr_;
            
            inline boost::asio::io_service& io_service() { return io_service_; }
            
            void terminate();
            bool initialize();
            bool prepare_for_run( const adcontrols::ControlMethod::Method& );
            bool prepare_for_run( const acqrscontrols::u5303a::method& );
            bool run();
            bool stop();
            bool trigger_inject_out();

            void connect( digitizer::command_reply_type f );
            void disconnect( digitizer::command_reply_type f );
            void connect( digitizer::waveform_reply_type f );
            void disconnect( digitizer::waveform_reply_type f );
            void setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw >& ptr );

            bool findResource();

            inline IAgMD2Ex2Ptr& spDriver() { return spDriver_; }

            inline const acqrscontrols::u5303a::method& method() const { return method_; }

            inline const acqrscontrols::u5303a::identify& ident() const { return *ident_; }

            inline std::shared_ptr< acqrscontrols::u5303a::identify > ident_ptr() { return ident_; }

            inline bool isSimulated() const { return simulated_; }
            
            void error_reply( const _com_error& e, const std::string& );

            inline void digitizerNumRecords( int numRecords ) { digitizerNumRecords_ = numRecords; }
            inline uint64_t digitizerNumRecords() const { return digitizerNumRecords_; }

            inline uint32_t dataSerialNumber( bool postInc = true ) { return postInc ? serialnumber_++ : serialnumber_; }

        private:
            static task * instance_;
            static std::mutex mutex_;

            uint64_t digitizerNumRecords_;

            IAgMD2Ex2Ptr spDriver_;
            std::vector< adportable::asio::thread > threads_;
            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;
            boost::asio::io_service::strand strand_;
            bool simulated_;
            acqrscontrols::u5303a::method method_;
            uint32_t serialnumber_;
            std::atomic<int> acquire_post_count_;
            uint64_t inject_timepoint_;
            std::vector< _bstr_t > foundResources_;

            std::deque< std::shared_ptr< SampleProcessor > > queue_;
            std::vector< digitizer::command_reply_type > reply_handlers_;
            std::vector< digitizer::waveform_reply_type > waveform_handlers_;
            std::shared_ptr< acqrscontrols::u5303a::identify > ident_;
            std::shared_ptr< adportable::TimeSquaredScanLaw > scanlaw_;

            bool handle_initial_setup( int nDelay, int nSamples, int nAverage );
            bool handle_terminating();
            bool handle_acquire();
            bool handle_prepare_for_run( const acqrscontrols::u5303a::method );
            bool handle_protocol( const acqrscontrols::u5303a::method );            
            bool acquire();
            bool waitForEndOfAcquisition( int timeout );
            bool readData( acqrscontrols::u5303a::waveform& );
        };

        const std::chrono::steady_clock::time_point task::uptime_ = std::chrono::steady_clock::now();
        const uint64_t task::tp0_ = std::chrono::duration_cast<std::chrono::nanoseconds>( task::uptime_.time_since_epoch() ).count();

    }
}

using namespace u5303a;
using namespace u5303a::detail;

task * task::instance_ = 0;
std::mutex task::mutex_;

digitizer::digitizer()
{
    boost::interprocess::shared_memory_object::remove( "waveform_simulator" );
}

digitizer::~digitizer()
{
}

bool
digitizer::peripheral_initialize()
{
    return task::instance()->initialize();
}

bool
digitizer::peripheral_prepare_for_run( const adcontrols::ControlMethod::Method& m )
{
    using adcontrols::ControlMethod::MethodItem;

    adcontrols::ControlMethod::Method cm( m );
    cm.sort();
    auto it = std::find_if( cm.begin(), cm.end(), [] ( const MethodItem& mi ){ return mi.modelname() == "u5303a"; } );
    if ( it != cm.end() ) {
        acqrscontrols::u5303a::method m;

        if ( it->get( *it, m ) ) {
            return task::instance()->prepare_for_run( m );
        }

    }
    return false;
}

bool
digitizer::peripheral_prepare_for_run( const acqrscontrols::u5303a::method& m )
{

    return task::instance()->prepare_for_run( m );
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
digitizer::peripheral_terminate()
{
    task::instance()->terminate();
    return true;
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

void
digitizer::setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw > ptr )
{
    task::instance()->setScanLaw( ptr );
}

task::task() : work_( io_service_ )
             , strand_( io_service_ )
             , simulated_( false )
             , serialnumber_( 0 )
             , acquire_post_count_( 0 )
             , exptr_( nullptr )
             , digitizerNumRecords_( 1 )
{
    for ( int i = 0; i < 2; ++i ) {

        threads_.push_back( adportable::asio::thread( [this]() {
                    try {
                        ::CoInitialize( 0 );
                        io_service_.run();
                    } catch ( ... ) {
                        ADERROR() << "Exception: " << boost::current_exception_diagnostic_information();
                        exptr_ = std::current_exception();
                    }
                    ::CoUninitialize();
                } ) );

    }
}

task::~task()
{
}

bool
task::findResource()
{
    bool found( false );

    IResourceManagerPtr rm;
    if ( rm.CreateInstance( __uuidof( ResourceManager ) ) == S_OK ) {

        try {
            if ( SAFEARRAY * list = rm->FindRsrc( "PXI?*" ) ) {
                
                safearray_t<BSTR> sa( list );
                
                for ( size_t i = 0; i < sa.size(); ++i ) {
                    
                    _bstr_t res = sa.data()[i];

                    if ( std::string( static_cast<const char *>( res ) ).find( "INSTR" ) != std::string::npos ) {

                        ADTRACE() << "IVI Resource on PXI device found: " << res;

                        IAgMD2Ex2Ptr spDriver;
                        if ( spDriver.CreateInstance( __uuidof( AgMD2 ) ) == S_OK ) {
                            if ( spDriver->Initialize( res, VARIANT_FALSE, VARIANT_FALSE, "DriverSetup= CAL=0" ) == S_OK ) {
                                try {
                                    _bstr_t model = spDriver->Identity->InstrumentModel;
                                    if ( model == _bstr_t( L"U5303A" ) )
                                        foundResources_.push_back( res );
                                    _bstr_t identifier = spDriver->Identity->Identifier;
                                    _bstr_t revision   = spDriver->Identity->Revision;
                                    _bstr_t vendor     = spDriver->Identity->Vendor;
                                    _bstr_t description= spDriver->Identity->Description;
                                    _bstr_t firmwareRevision = spDriver->Identity->InstrumentFirmwareRevision;
                                    
                                    ADTRACE() << "Found: " << res 
                                              << "; model '" << model << "' "
                                              << identifier << "; "
                                              << revision << "; "
                                              << vendor << "; "
                                              << description << "; "
                                              << firmwareRevision << "; ";
                                } catch ( _com_error& e ) {
                                    ADERROR() << "Exception: " << e.Description() << ", " << e.ErrorMessage();                                    
                                }
                                spDriver->Close();
                            }
                        }
                    }
                }
                SafeArrayDestroy( list );
            }
        } catch ( _com_error& e ) {
            ADERROR() << "Exception: " << e.Description() << ", " << e.ErrorMessage();
        }
    }
    return !foundResources_.empty();
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

	io_service_.post( strand_.wrap( [this] { findResource(); } ) );

    io_service_.post( strand_.wrap( [this] { handle_initial_setup( 32, 1024 * 10, 2 ); } ) );
        
    return true;
}

bool
task::prepare_for_run( const acqrscontrols::u5303a::method& method )
{
    auto& m = method.method_;

    ADTRACE() << "u5303a::task::prepare_for_run";
    ADTRACE() << "\tfront_end_range: " << m.front_end_range << "\tfrontend_offset: " << m.front_end_offset
        << "\text_trigger_level: " << m.ext_trigger_level
        << "\tsamp_rate: " << m.samp_rate
        << "\tnbr_of_samples: " << m.nbr_of_s_to_acquire_ << "; " << m.digitizer_nbr_of_s_to_acquire
        << "\tnbr_of_average: " << m.nbr_of_averages
        << "\tdelay_to_first_s: " << adcontrols::metric::scale_to_micro( m.digitizer_delay_to_first_sample )
        << "\tinvert_signal: " << m.invert_signal
        << "\tnsa: " << m.nsa;
    
    io_service_.post( strand_.wrap( [=] { handle_prepare_for_run( method ); } ) );

    if ( acquire_post_count_ == 0 ) {
        acquire_post_count_++;
        io_service_.post( strand_.wrap( [this] { handle_acquire(); } ) );
	}

    return true;
}


bool
task::run()
{
    // std::lock_guard< std::mutex > lock( mutex_ );
	if ( queue_.empty() ) {
        queue_.push_back( std::make_shared< SampleProcessor >( io_service_ ) );
        queue_.back()->prepare_storage( 0 ); //pMasterObserver_->_this() );
    }
	// status_current_ = ControlServer::ePreparingForRun;
	// status_being_ = ControlServer::eReadyForRun;
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
	(void)nAverage;
	(void)nSamples;
	(void)nDelay;
	// BSTR strResourceDesc = bstr_t(L"PXI4::0::0::INSTR");

    if ( spDriver_.CreateInstance( __uuidof( AgMD2 ) ) != S_OK )
        return false;

    // If desired, use 'DriverSetup= CAL=0' to prevent digitizer from doing a SelfCal (~1 seconds) each time
    // it is initialized or reset which is the default behavior. By default set to false.
    // CUSTOM FIRMWARE NAME: U5303ADPULX2AVE.bit

    // note that CAL=0 is not necessary with MD2 as the card is initialized without calibration.
				
    VARIANT_BOOL idQuery = VARIANT_TRUE;
    VARIANT_BOOL reset   = VARIANT_TRUE;
    bool simulated = false;
    bool success = false;

    BSTR strInitOptions = _bstr_t( L"Simulate=false, DriverSetup= Model=U5303A" );

    if ( auto p = getenv( "AcqirisOption" ) ) {
        if ( p && std::strcmp( p, "simulate" ) == 0 ) {
            strInitOptions = _bstr_t( L"Simulate=true, DriverSetup= Model=U5303A, Trace=false" );
            simulated = true;
            foundResources_.push_back( _bstr_t( L"PXI3::0::0::INSTR" ) );
        }
    }

    for ( auto& res : foundResources_ ) {
        try {
            ADTRACE() << "Initialize resource: " << res;
            success = spDriver_->Initialize( res, VARIANT_TRUE, VARIANT_TRUE, strInitOptions ) == S_OK;
            ADTRACE() << "Success initialize " << res;
            break;
        } catch ( _com_error& e ) {
            ERR(e, (boost::format("; while Initialize %1%") % static_cast<const char *>( res ) ).str() );
        }
    }

    if ( success ) {
        simulated_ = simulated;
        
        ident_ = std::make_shared< acqrscontrols::u5303a::identify >();

        try {
            ident_->Identifier() = static_cast<const char *>( _bstr_t( spDriver_->Identity->Identifier.GetBSTR() ) );
            ident_->Revision() = static_cast<const char *>( _bstr_t( spDriver_->Identity->Revision.GetBSTR() ) );
            ident_->Vendor() = static_cast<const char *>( _bstr_t( spDriver_->Identity->Vendor.GetBSTR() ) );
            ident_->Description() = static_cast<const char *>( _bstr_t( spDriver_->Identity->Description.GetBSTR() ) );
            ident_->InstrumentModel() = static_cast<const char *>( _bstr_t( spDriver_->Identity->InstrumentModel.GetBSTR() ) );
            ident_->FirmwareRevision() = static_cast<const char *>( _bstr_t( spDriver_->Identity->InstrumentFirmwareRevision.GetBSTR() ) );
            if ( auto iinfo = spDriver_->InstrumentInfo ) {
                ident_->SerialNumber() = iinfo->SerialNumberString;
                ident_->IOVersion() = iinfo->IOVersion;
                ident_->Options() = iinfo->Options; // Options_ = "CH2,LX2,F05,INT,M02,SR2"; "CH2,LX2,F05,AVG,DGT,M02,SR1"

                ident_->NbrADCBits() = iinfo->NbrADCBits;
            }
            // SR0 = 0.5GS/s 2ch; SR0+INT = 1.0GS/s 1ch;
            // SR1 = 1.0GS/s 2ch; SR1+INT = 2.0GS/s 1ch;
            // SR2 = 1.6GS/s 2ch; SR2+INT = 3.2GS/s 1ch;
            // M02 = 256MB; M10 = 1GB, M40 = 4GB
            
        } catch ( _com_error& ex ) {
            ERR( ex, "Identification failed." );
        }

        for ( auto& reply : reply_handlers_ ) reply( "Identifier", ident_->Identifier() );
        for ( auto& reply : reply_handlers_ ) reply( "Revision", ident_->Revision() );
        for ( auto& reply : reply_handlers_ ) reply( "Description", ident_->Description() );
        for ( auto& reply : reply_handlers_ ) reply( "InstrumentModel", ident_->InstrumentModel() );
        for ( auto& reply : reply_handlers_ ) reply( "InstrumentFirmwareRevision", ident_->FirmwareRevision() );
        for ( auto& reply : reply_handlers_ ) reply( "SerialNumber", ident_->SerialNumber() );
        for ( auto& reply : reply_handlers_ ) reply( "IOVersion", ident_->IOVersion() );
        for ( auto& reply : reply_handlers_ ) reply( "Options", ident_->Options() );

        try {
            if ( method_.mode_ == 0 )
                device<Digitizer>::initial_setup( *this, method_, ident().Options() );
            else
                device<Averager>::initial_setup( *this, method_, ident().Options() );
        } catch ( _com_error& e ) {
            ADERROR() << "Exception: " << e.Description() << ", " << e.ErrorMessage();
        }

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
task::handle_prepare_for_run( const acqrscontrols::u5303a::method m )
{
    if ( m.mode_ == 0 )
        device<Digitizer>::initial_setup( *this, m, ident().Options() );
    else
        device<Averager>::initial_setup( *this, m, ident().Options() );

    if ( m.mode_ && simulated_ ) {
        acqrscontrols::u5303a::method a( m );
        a.method_.samp_rate = spDriver()->Acquisition2->SampleRate;
        simulator::instance()->setup( a );
    }

    method_ = m;

    return true;
}

bool
task::handle_protocol( const acqrscontrols::u5303a::method m )
{
    if ( m.mode_ == 0 )
        device<Digitizer>::setup( *this, m );
    else
        device<Averager>::setup( *this, m );

    if ( m.mode_ && simulated_ )
        simulator::instance()->setup( m );
    
    method_ = m;
    return true;
}


bool
task::handle_acquire()
{
    static int counter_;

    ++acquire_post_count_;
    io_service_.post( strand_.wrap( [&] { handle_acquire(); } ) );    // scedule for next acquire

    --acquire_post_count_;
    if ( acquire() ) {
        if ( waitForEndOfAcquisition( 3000 ) ) {
            if ( method_.mode_ == 0 ) { // digitizer
                std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > > vec;
                device<Digitizer>::readData( *this, digitizerNumRecords_, vec );
                for ( auto& waveform: vec ) {
                    acqrscontrols::u5303a::method m;
                    for ( auto& reply: waveform_handlers_ ) {
                        if ( reply( waveform.get(), nullptr, m ) )
                            handle_protocol( m );
                    }
                }
            } else {
                uint32_t events( 0 );
                auto waveform = std::make_shared< acqrscontrols::u5303a::waveform >( ident_, events );
                if ( readData( *waveform ) ) {
                    acqrscontrols::u5303a::method m;
                    for ( auto& reply : waveform_handlers_ ) {
                        if ( reply( waveform.get(), nullptr, m ) )
                            handle_protocol( m );
                    }
                }
            }
        }
        return true;
    }
    ADTRACE() << "===== handle_acquire waitForEndOfAcquisitioon == not handled.";
    return false;
}

bool
task::acquire()
{
    if ( method_.mode_ && simulated_ )    
        return device<Simulate>::acquire( *this );
        
    if ( method_.mode_ == 0 ) {
        return device<Digitizer>::acquire( *this );
    }  else {
        return device<Averager>::acquire( *this );
    }
}

bool
task::waitForEndOfAcquisition( int timeout )
{
    if ( method_.mode_ && simulated_ ) {
        std::this_thread::sleep_for( std::chrono::microseconds( 500 ) );
        return device<Simulate>::waitForEndOfAcquisition( *this, timeout );
    }

    if ( method_.mode_ == 0 ) {
        if ( simulated_ )
            std::this_thread::sleep_for( std::chrono::microseconds( 500 ) );
        return device<Digitizer>::waitForEndOfAcquisition( *this, timeout );
    } else {
        return device<Averager>::waitForEndOfAcquisition( *this, timeout );
    }
}

bool
task::readData( acqrscontrols::u5303a::waveform& data )
{
    data.serialnumber_ = serialnumber_++;

    if ( method_.mode_ && simulated_ )    
        return device<Simulate>::readData( *this, data );

    if ( method_.mode_ == 0 ) {
        return device<Digitizer>::readData( *this, data );
    } else {
        return device<Averager>::readData( *this, data );
    }
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
    // no way to do 'disconnect' since std::function<> has no operator == implemented.
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
    // no way to do 'disconnect' since std::function<> has no operator == implemented.
}

void
task::error_reply( const _com_error& e, const std::string& method )
{
    _bstr_t msg( _bstr_t(L"Error: ") + e.Description() + L": " + e.ErrorMessage() );
    for ( auto& reply: reply_handlers_ )
        reply( method, static_cast< const char *>( msg ) );
}

void
task::setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw >& ptr )
{
    scanlaw_ = ptr;
}

template<> bool
device<Averager>::initial_setup( task& task, const acqrscontrols::u5303a::method& m, const std::string& options )
{
    // Create smart pointers to Channels and TriggerSource interfaces
    IAgMD2ChannelPtr spCh1 = task.spDriver()->Channels->Item[L"Channel1"];
    
    // Set Interleave ON
    if ( options.find( "INT" ) != options.npos ) {
        try { spCh1->TimeInterleavedChannelList = "Channel2"; } catch ( _com_error& e ) {
            TERR( e, "TimeInterleavedChannelList" ); }
    }

    try {
        spCh1->PutRange( m.method_.front_end_range );
        spCh1->PutOffset(m.method_.front_end_offset);
        task.spDriver()->Channels2->Item2[L"Channel1"]->DataInversionEnabled = m.method_.invert_signal ? VARIANT_TRUE : VARIANT_FALSE;

    } catch (_com_error& e) {
        TERR( e, "device<Averager>::initial_setup" );
        
	}

    // Setup triggering
    try {
        task.spDriver()->Trigger->ActiveSource = "External1";
        task.spDriver()->Trigger->Delay = m.method_.digitizer_delay_to_first_sample;

        IAgMD2TriggerSourcePtr spTrigSrc = task.spDriver()->Trigger->Sources->Item[ L"External1" ];
        spTrigSrc->Level = m.method_.ext_trigger_level;
        spTrigSrc->Edge->Slope = AgMD2TriggerSlopePositive;

    } catch ( _com_error& e ) {
        TERR( e, "device<Averager>::initial_setup" );        
    }

        
    try {
        // Calibrate

        ADTRACE() << "Set the Mode Averager...";
        task.spDriver()->Acquisition2->Mode = AgMD2AcquisitionModeAverager;

    } catch ( _com_error& e ) {
        TERR( e, "Acquisition::Mode" );
    }

    // Set the sample rate and nbr of samples to acquire
    bool success = false;
    try {
        task.spDriver()->Acquisition2->SampleRate = m.method_.samp_rate;
        success = true;
    }  catch (_com_error& e) {
        TERR(e,"SampleRate");
    }

    if ( !success ) {
        try {
            task.spDriver()->Acquisition2->SampleRate = adportable::compare<double>::approximatelyEqual( m.method_.samp_rate, 1.0e9 ) ? 3.2e9 : 1.0e9;
            success = true;
        } catch ( _com_error& e ) {
            TERR( e, "device<Averager>::initial_setup" );            
        }
    }

// #if 0
//     task.spDriver()->Channels2->GetItem2("Channel1")->Filter->Bypass = 1;
// #endif

    try {
        task.spDriver()->Acquisition->RecordSize = m.method_.digitizer_nbr_of_s_to_acquire;
        task.spDriver()->Acquisition->NumRecordsToAcquire = 1;
        task.spDriver()->Acquisition2->NumberOfAverages = m.method_.nbr_of_averages;
        task.spDriver()->Acquisition2->Mode = AgMD2AcquisitionModeAverager;

        ADTRACE() << "Calibrating...";
        task.spDriver()->Calibration->SelfCalibrate();
    } catch ( _com_error& e ) {
        TERR( e, "device<Averager>::initial_setup" );        
    }

	return true;
}

template<> bool
device<Digitizer>::initial_setup( task& task, const acqrscontrols::u5303a::method& m, const std::string& options )
{
    IAgMD2ChannelPtr spCh1 = task.spDriver()->Channels->Item[ L"Channel1" ];

    // Set Interleave ON
    if ( options.find( "INT" ) != options.npos ) {
        try { spCh1->TimeInterleavedChannelList = "Channel2"; } catch ( _com_error& e ) {
            TERR( e, "TimeInterleavedChannelList" ); }
    }
    try {
        spCh1->PutRange( m.method_.front_end_range );
        spCh1->PutOffset(m.method_.front_end_offset);
        task.spDriver()->Channels2->Item2[L"Channel1"]->DataInversionEnabled = m.method_.invert_signal ? VARIANT_TRUE : VARIANT_FALSE;

        // Setup triggering
        task.spDriver()->Trigger->ActiveSource = "External1";
        task.spDriver()->Trigger->Delay = m.method_.digitizer_delay_to_first_sample;

        IAgMD2TriggerSourcePtr spTrigSrc = task.spDriver()->Trigger->Sources->Item[ L"External1" ];
        spTrigSrc->Level = m.method_.ext_trigger_level;
        spTrigSrc->Edge->Slope = AgMD2TriggerSlopePositive;

    } catch ( _com_error& e ) {
        TERR( e, "device<Digitizer>::initial_setup" );
    }        

    // Set the sample rate and nbr of samples to acquire
    bool success = false;
    
    double max_rate = 3.2e9;
    if ( options.find( "SR1" ) != options.npos ) {
        max_rate = 1.0e9;
        if ( options.find( "INT" ) != options.npos )
            max_rate = 2.0e9;
    } else if ( options.find( "SR2" ) != options.npos ) {
        max_rate = 1.6e9;
        if ( options.find( "INT" ) != options.npos )
            max_rate = 3.2e9;
    }
    
    for ( auto samp_rate : { m.method_.samp_rate, max_rate } ) {
        try {
            task.spDriver()->Acquisition2->SampleRate = samp_rate;
            success = true;
            break;
        } catch ( _com_error& e ) {
            TERR( e, "SampleRate" );
        }
    }
        
// #if 0
//     task.spDriver()->Channels2->GetItem2("Channel1")->Filter->Bypass = 1;
// #endif

    //--->
    try {
        task.digitizerNumRecords( m.method_.nbr_records );
        task.spDriver()->Acquisition->RecordSize = m.method_.digitizer_nbr_of_s_to_acquire;
        task.spDriver()->Acquisition->NumRecordsToAcquire = task.digitizerNumRecords();

        ADTRACE() << "Set the Mode Digitizer; RecordSize=" << task.spDriver()->Acquisition->RecordSize << "; NumRecords=" << task.spDriver()->Acquisition->NumRecordsToAcquire;
        task.spDriver()->Acquisition2->Mode = AgMD2AcquisitionModeNormal;

        // Calibrate
        ADTRACE() << "Calibrating...";
        task.spDriver()->Calibration->SelfCalibrate();

    } catch ( _com_error& e ) {
        TERR(e,"mbr_of_s_to_acquire");        
    }

	return true;
}

template<> bool
device<Averager>::setup( task& task, const acqrscontrols::u5303a::method& m )
{
    IAgMD2ChannelPtr spCh1 = task.spDriver()->Channels->Item[L"Channel1"];
#if 0
    // protocol paramater -- this can not be used due to U5303A require 'calibration' when change number of samples and/or number of averages.
    try {
        uint64_t nDelay = uint64_t( m.delay_to_first_sample / 1.0e-9 + 0.5 ); // ns
        task.spDriver()->Trigger->Delay = double( ( nDelay / 16 ) * 16 ) * 1.0e-9;
    } catch ( _com_error& e ) {
        TERR(e,"mode");        
    }
    // task.spDriver()->Calibration->SelfCalibrate();
    try {
        task.spDriver()->Acquisition->RecordSize = m.nbr_of_s_to_acquire;
    } catch ( _com_error& e ) {
        TERR(e,"mbr_of_s_to_acquire");        
    }
    try {
        task.spDriver()->Acquisition2->NumberOfAverages = m.nbr_of_averages;
    } catch ( _com_error& e ) {
        TERR(e,"mbr_of_averages");        
    }
#endif
    return true;
}

template<> bool
device<Digitizer>::setup( task& task, const acqrscontrols::u5303a::method& m )
{
    return true;
}

template<> bool
device<Averager>::acquire( task& task )
{
    // Perform the acquisition.
    try {
        task.spDriver()->Acquisition->Initiate();
    } catch ( _com_error & e ) {
        TERR(e,"Initialte");
    } 
    return true;
}

template<> bool
device<Digitizer>::acquire( task& task )
{
    // Perform the acquisition.
    try {
        task.spDriver()->Acquisition->Initiate();
    } catch ( _com_error & e ) {
        TERR(e,"Initialte");
    } 
    return true;
}

template<> bool
device<Averager>::waitForEndOfAcquisition( task& task, int timeout )
{
	(void)timeout;

    long const timeoutInMs = 3000;

    try {
        task.spDriver()->Acquisition->WaitForAcquisitionComplete(timeoutInMs);
    } catch ( _com_error& e ) {
        TERR(e, "WaitForAcquisitionComplete");
    }
    return true;
}

template<> bool
device<Digitizer>::waitForEndOfAcquisition( task& task, int timeout )
{
	(void)timeout;

    long const timeoutInMs = 3000;

    try {
        task.spDriver()->Acquisition->WaitForAcquisitionComplete(timeoutInMs);
    } catch ( _com_error& e ) {
        TERR(e, "WaitForAcquisitionComplete");
    }
    return true;
}

template<> bool
device<Digitizer>::readData( task& task, acqrscontrols::u5303a::waveform& data )
{
    IAgMD2Channel2Ptr spCh1 = task.spDriver()->Channels2->Item2[ L"Channel1" ];
    SAFEARRAY* dataArray = 0;
    __int64 firstValidPoint = 0;
    double initialXTimeSeconds = 0;
    double initialXTimeFraction = 0;

    const int64_t numPointsPerRecord = task.method().method_.digitizer_nbr_of_s_to_acquire;
    
    try {
        spCh1->Measurement2->FetchWaveformInt16( &dataArray
                                                 , &data.meta_.actualPoints
                                                 , &firstValidPoint
                                                 , &data.meta_.initialXOffset
                                                 , &initialXTimeSeconds
                                                 , &initialXTimeFraction
                                                 , &data.meta_.xIncrement
                                                 , &data.meta_.scaleFactor
                                                 , &data.meta_.scaleOffset );

        auto tp = std::chrono::steady_clock::now();
        data.timeSinceEpoch_ = std::chrono::duration_cast<std::chrono::nanoseconds>( tp.time_since_epoch() ).count();

        data.method_ = task.method();

        data.meta_.actualAverages = 0; // digitizer

        data.meta_.initialXTimeSeconds = initialXTimeSeconds + initialXTimeFraction;
        if ( data.meta_.initialXTimeSeconds == 0 ) {  // bitwise zero though this is double
            data.meta_.initialXTimeSeconds = double( std::chrono::duration_cast<std::chrono::nanoseconds>( tp - task::uptime_ ).count() ) * 1.0e-9;
        }

        safearray_t<int16_t> sa( dataArray );

        data.resize( data.meta_.actualPoints );

        std::copy( sa.data() + firstValidPoint, sa.data() + data.meta_.actualPoints, data.begin() );

        // Release memory.
        SafeArrayDestroy(dataArray);

    } catch ( _com_error& e ) {
        TERR(e,"readData::FetchWaveformInt16");
        return false;
    }
    return true;
}

template<> bool
device<Digitizer>::readData( task& task, uint64_t numRecords, std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > >& vec )
{
    IAgMD2Channel2Ptr spCh1 = task.spDriver()->Channels2->Item2[ L"Channel1" ];
    SAFEARRAY* dataArray( 0 );
    SAFEARRAY* firstValidPoints( 0 );
    SAFEARRAY* actualPoints( 0 );
    SAFEARRAY* initialXOffset( 0 );
    SAFEARRAY* initialXTimeSeconds( 0 );
    SAFEARRAY* initialXTimeFraction( 0 );
    int64_t firstRecord( 0 ), offsetWithinRecord( 0 ), actualRecords(0);
    double xIncrement( 0 ), scaleFactor( 0 ), scaleOffset( 0 );
    const int64_t numPointsPerRecord = task.method().method_.digitizer_nbr_of_s_to_acquire;
    
    try {

        spCh1->MultiRecordMeasurement->FetchMultiRecordWaveformInt16( firstRecord
                                                                      , numRecords
                                                                      , offsetWithinRecord
                                                                      , numPointsPerRecord
                                                                      , &dataArray
                                                                      , &actualRecords
                                                                      , &actualPoints
                                                                      , &firstValidPoints // --> data.meta_
                                                                      , &initialXOffset // --> data.meta_
                                                                      , &initialXTimeSeconds
                                                                      , &initialXTimeFraction
                                                                      , &xIncrement
                                                                      , &scaleFactor
                                                                      , &scaleOffset );
    } catch ( _com_error& e ) {
        TERR(e,"readMultiData::FetchMultiRecordWaveformInt16");
        return false;
    }
    
    auto tp = std::chrono::steady_clock::now();
    safearray_t<__int64> saFirstValidPoint( firstValidPoints );
    // safearray_t<int> saFlags( flags );
    safearray_t< double > saInitialXTimeSeconds( initialXTimeSeconds );
    safearray_t< double > saInitialXTimeFraction( initialXTimeFraction );
    safearray_t< int64_t > saActualPoints( actualPoints );
    safearray_t< int16_t > saData( dataArray );

    for ( int64_t iRecord = 0; iRecord < actualRecords; ++iRecord ) {

        if ( auto data = std::make_shared< acqrscontrols::u5303a::waveform >(task.ident_ptr(), task.dataSerialNumber() ) ) {

            data->timeSinceEpoch_ = std::chrono::duration_cast<std::chrono::nanoseconds>( tp.time_since_epoch() ).count();
            data->method_ = task.method();
            data->meta_.actualAverages = 0; // digitizer
            data->meta_.actualPoints = saActualPoints.data()[ iRecord ];
            data->meta_.initialXTimeSeconds = saInitialXTimeSeconds.data()[ iRecord ] + saInitialXTimeFraction.data()[ iRecord ];
            data->meta_.xIncrement = xIncrement;
            data->meta_.scaleFactor = scaleFactor;
            data->meta_.scaleOffset = scaleOffset;

            data->resize( data->meta_.actualPoints );
            
            int64_t firstValidPoint = saFirstValidPoint.data()[ iRecord ];

            std::copy( saData.data() + firstValidPoint, saData.data() + firstValidPoint + data->meta_.actualPoints, data->begin() );

            vec.push_back( data );
        }
    }

    SafeArrayDestroy( dataArray );
    SafeArrayDestroy( firstValidPoints );
    SafeArrayDestroy( actualPoints );
    SafeArrayDestroy( initialXOffset );
    SafeArrayDestroy( initialXTimeSeconds );
    SafeArrayDestroy( initialXTimeFraction );

    return true;
}


template<> bool
device<Averager>::readData( task& task, acqrscontrols::u5303a::waveform& data )
{
    IAgMD2Channel2Ptr spCh1 = task.spDriver()->Channels2->Item2[ L"Channel1" ];
    __int64 firstRecord = 0;
    __int64 numRecords = 1;
    __int64 offsetWithinRecord = 0;
    SAFEARRAY* dataArray = 0;
    long actualAverages = 0;
    // __int64 actualRecords = 0;
    SAFEARRAY* actualPoints = 0;
    SAFEARRAY* firstValidPoints = 0;
    SAFEARRAY* initialXTimeSeconds = 0;
    SAFEARRAY* initialXTimeFraction = 0;
    SAFEARRAY* flags = 0;
    const int64_t numPointsPerRecord = task.method().method_.digitizer_nbr_of_s_to_acquire;
    
    try {
        spCh1->Measurement2->FetchAccumulatedWaveformInt32( firstRecord
                                                            , numRecords
                                                            , offsetWithinRecord
                                                            , numPointsPerRecord
                                                            , &dataArray
                                                            , &actualAverages
                                                            , &data.meta_.actualRecords
                                                            , &actualPoints
                                                            , &firstValidPoints
                                                            , &data.meta_.initialXOffset
                                                            , &initialXTimeSeconds
                                                            , &initialXTimeFraction
                                                            , &data.meta_.xIncrement
                                                            , &data.meta_.scaleFactor
                                                            , &data.meta_.scaleOffset
                                                            , &flags );

        data.timeSinceEpoch_ = std::chrono::steady_clock::now().time_since_epoch().count();

        data.method_ = task.method();

        data.meta_.actualAverages = actualAverages;

		safearray_t<__int64> saFirstValidPoint( firstValidPoints );
        __int64 firstValidPoint = saFirstValidPoint.data()[ 0 ];
        
        safearray_t<int> saFlags( flags );
        data.meta_.flags = saFlags.data()[ 0 ];

        safearray_t< double > saInitialXTimeSeconds( initialXTimeSeconds );
        safearray_t< double > saInitialXTimeFraction( initialXTimeFraction );
        data.meta_.initialXTimeSeconds = saInitialXTimeSeconds.data()[ 0 ] + saInitialXTimeFraction.data()[ 0 ];

		safearray_t<int32_t> sa( dataArray );
        auto dp = data.data( numPointsPerRecord );
        std::copy( sa.data() + firstValidPoint, sa.data() + numPointsPerRecord, dp );

        // Release memory.
        SafeArrayDestroy(flags);
        SafeArrayDestroy(initialXTimeFraction);
        SafeArrayDestroy(initialXTimeSeconds);
        SafeArrayDestroy(firstValidPoints);
        SafeArrayDestroy(actualPoints);
        SafeArrayDestroy(dataArray);

    } catch ( _com_error& e ) {
        TERR(e,"readData::ReadIndirectInt32");
        return false;
    }
    return true;
}


template<> bool
device<Simulate>::initial_setup( task& task, const acqrscontrols::u5303a::method& m, const std::string& options )
{
    simulator::instance()->setup( m );
    return true;
}

template<> bool
device<Simulate>::setup( task& task, const acqrscontrols::u5303a::method& m )
{
    simulator::instance()->setup( m );
    return true;
}

template<> bool
device<Simulate>::acquire( task& task )
{
    return simulator::instance()->acquire();
}

template<> bool
device<Simulate>::waitForEndOfAcquisition( task& task, int /* timeout */)
{
    return simulator::instance()->waitForEndOfAcquisition();
}

template<> bool
device<Simulate>::readData( task& task, acqrscontrols::u5303a::waveform& data )
{
    simulator::instance()->readData( data );
    data.timeSinceEpoch_ = std::chrono::steady_clock::now().time_since_epoch().count();
    return true;
}

