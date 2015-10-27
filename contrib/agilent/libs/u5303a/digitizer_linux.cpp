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
#include "agmd2.hpp"
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

namespace u5303a {

    namespace detail {
 
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

            inline AgMD2 * spDriver() { return spDriver_.get(); }

            inline const acqrscontrols::u5303a::method& method() const { return method_; }

            inline const acqrscontrols::u5303a::identify& ident() const { return *ident_; }

            inline std::shared_ptr< acqrscontrols::u5303a::identify > ident_ptr() { return ident_; }

            inline bool isSimulated() const { return simulated_; }
            
            void error_reply( const std::string& emsg, const std::string& );

        private:
            friend std::unique_ptr< task >::deleter_type;
            static std::unique_ptr< task > instance_;
            static std::mutex mutex_;

            uint64_t digitizerNumRecords_;

            std::shared_ptr< AgMD2 > spDriver_;
            std::vector< adportable::asio::thread > threads_;
            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;
            boost::asio::io_service::strand strand_;
            bool simulated_;
            acqrscontrols::u5303a::method method_;
            std::atomic<int> acquire_post_count_;
            uint64_t inject_timepoint_;
            std::vector< std::string > foundResources_;

            std::deque< std::shared_ptr< SampleProcessor > > queue_;
            std::vector< digitizer::command_reply_type > reply_handlers_;
            std::vector< digitizer::waveform_reply_type > waveform_handlers_;
            std::shared_ptr< acqrscontrols::u5303a::identify > ident_;
            std::shared_ptr< adportable::TimeSquaredScanLaw > scanlaw_;

            bool handle_initial_setup();
            bool handle_terminating();
            bool handle_acquire();
            bool handle_prepare_for_run( const acqrscontrols::u5303a::method );
            bool handle_protocol( const acqrscontrols::u5303a::method );            
            bool acquire();
            bool waitForEndOfAcquisition( int timeout );
            bool readData( acqrscontrols::u5303a::waveform& );
        };

        struct device {
            static bool initial_setup( task&, const acqrscontrols::u5303a::method&, const std::string& options );
            static bool setup( task&, const acqrscontrols::u5303a::method& );
            static bool acquire( task& );
            static bool waitForEndOfAcquisition( task&, int timeout );
            // static bool readData( task&, uint64_t numRecords, std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > >& );
            // static bool readData16( task&, acqrscontrols::u5303a::waveform& );
            // static bool readData32( task&, acqrscontrols::u5303a::waveform& );            
        };

        const std::chrono::steady_clock::time_point task::uptime_ = std::chrono::steady_clock::now();
        const uint64_t task::tp0_ = std::chrono::duration_cast<std::chrono::nanoseconds>( task::uptime_.time_since_epoch() ).count();

    }
}

using namespace u5303a;
using namespace u5303a::detail;

std::unique_ptr< task > task::instance_;

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
             , acquire_post_count_( 0 )
             , exptr_( nullptr )
             , digitizerNumRecords_( 1 )
{
    for ( int i = 0; i < 2; ++i ) {

        threads_.push_back( adportable::asio::thread( [this]() {
                    try {
                        io_service_.run();
                    } catch ( ... ) {
                        ADERROR() << "Exception: " << boost::current_exception_diagnostic_information();
                        exptr_ = std::current_exception();
                    }
                } ) );

    }
}

task::~task()
{
    if ( !threads_.empty() )
        terminate();
}

bool
task::findResource()
{
    // workaround
    foundResources_.clear();
    foundResources_.push_back( "PXI3::0::0::INSTR" );
    foundResources_.push_back( "PXI4::0::0::INSTR" );
    foundResources_.push_back( "PXI0::0::0::INSTR" );
    foundResources_.push_back( "PXI1::0::0::INSTR" );
    return true;
}

task *
task::instance()
{
    static std::once_flag flag;
    std::call_once( flag, [](){ instance_.reset( new task() ); } );
    return instance_.get();
}

bool
task::initialize()
{
    ADTRACE() << "u5303a digitizer initializing...";

	io_service_.post( strand_.wrap( [this] { findResource(); } ) );

    io_service_.post( strand_.wrap( [this] { handle_initial_setup(); } ) );
        
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
task::handle_initial_setup()
{
    spDriver_ = std::make_shared< AgMD2 >();

    ViBoolean idQuery = VI_TRUE;
    ViBoolean reset   = VI_TRUE;
    bool simulated = false;
    bool success = false;

    const char * strInitOptions = "Simulate=false, DriverSetup= Model=U5303A";

    if ( auto p = getenv( "AcqirisOption" ) ) {
        if ( p && std::strcmp( p, "simulate" ) == 0 ) {
            strInitOptions = "Simulate=true, DriverSetup= Model=U5303A";
            simulated = true;
            success = spDriver_->InitWithOptions( "PXI40::0::0::INSTR", VI_TRUE, VI_TRUE, strInitOptions );
        }
    }

    if ( !simulated ) {
        for ( auto& res : foundResources_ ) {
            ADTRACE() << "Initialize resource: " << res;
            if ( success = spDriver_->InitWithOptions( res.c_str(), VI_TRUE, VI_TRUE, strInitOptions ) )
                break;
        }
    }

    if ( success ) {
        simulated_ = simulated;
        
        ident_ = std::make_shared< acqrscontrols::u5303a::identify >();
        spDriver_->Identify( ident_ );
        // SR0 = 0.5GS/s 2ch; SR0+INT = 1.0GS/s 1ch;
        // SR1 = 1.0GS/s 2ch; SR1+INT = 2.0GS/s 1ch;
        // SR2 = 1.6GS/s 2ch; SR2+INT = 3.2GS/s 1ch;
        // M02 = 256MB; M10 = 1GB, M40 = 4GB
        
        for ( auto& reply : reply_handlers_ ) reply( "Identifier", ident_->Identifier() );
        for ( auto& reply : reply_handlers_ ) reply( "Revision", ident_->Revision() );
        for ( auto& reply : reply_handlers_ ) reply( "Description", ident_->Description() );
        for ( auto& reply : reply_handlers_ ) reply( "InstrumentModel", ident_->InstrumentModel() );
        for ( auto& reply : reply_handlers_ ) reply( "InstrumentFirmwareRevision", ident_->FirmwareRevision() );
        for ( auto& reply : reply_handlers_ ) reply( "SerialNumber", ident_->SerialNumber() );
        for ( auto& reply : reply_handlers_ ) reply( "IOVersion", ident_->IOVersion() );
        for ( auto& reply : reply_handlers_ ) reply( "Options", ident_->Options() );

        device::initial_setup( *this, method_, ident().Options() );
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
    device::initial_setup( *this, m, ident().Options() );

    if ( m.mode_ && simulated_ ) {
        acqrscontrols::u5303a::method a( m );
        a.method_.samp_rate = spDriver()->SampleRate();
        simulator::instance()->setup( a );
    }

    method_ = m;

    return true;
}

bool
task::handle_protocol( const acqrscontrols::u5303a::method m )
{
    if ( m.mode_ == 0 )
        device::setup( *this, m );
    else
        device::setup( *this, m );

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
                digitizer::readData( *spDriver(), method_, vec );
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
        return simulator::instance()->acquire();
        
    return device::acquire( *this );
}

bool
task::waitForEndOfAcquisition( int timeout )
{
    if ( simulated_ ) {
        std::this_thread::sleep_for( std::chrono::microseconds( 100 ) );
        if ( method_.mode_ )
            return simulator::instance()->waitForEndOfAcquisition();
    }

    return device::waitForEndOfAcquisition( *this, timeout );
}

bool
task::readData( acqrscontrols::u5303a::waveform& data )
{
    data.serialnumber_ = spDriver()->dataSerialNumber();
    
    if ( method_.mode_ && simulated_ ) {
        simulator::instance()->readData( data );
        data.timeSinceEpoch_ = std::chrono::steady_clock::now().time_since_epoch().count();
        return true;        
    }

    if ( method_.mode_ == 0 ) {
        return digitizer::readData16( *spDriver(), method_, data );
    } else {
        return digitizer::readData32( *spDriver(), method_, data );
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
task::error_reply( const std::string& e, const std::string& method )
{
    for ( auto& reply: reply_handlers_ )
        reply( method, e.c_str() );
}

void
task::setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw >& ptr )
{
    scanlaw_ = ptr;
}

bool
device::initial_setup( task& task, const acqrscontrols::u5303a::method& m, const std::string& options )
{
    ViStatus rcode;
    ViInt32 const coupling = AGMD2_VAL_VERTICAL_COUPLING_DC;
    
    if ( options.find( "INT" ) != options.npos ) {
        // Set Interleave ON
        task.spDriver()->ConfigureTimeInterleavedChannelList( "Channel1", "Channel2" );
    }

    rcode = AgMD2_ConfigureChannel( task.spDriver()->session(), "Channel1", m.method_.front_end_range, m.method_.front_end_offset, coupling, VI_TRUE );
    task.spDriver()->log( rcode, __FILE__, __LINE__ );

    task.spDriver()->setActiveTriggerSource( "External1" );
    task.spDriver()->setTriggerLevel( "External1", m.method_.ext_trigger_level );
    task.spDriver()->setTriggerSlope( "External1", AGMD2_VAL_POSITIVE );
    task.spDriver()->setTriggerDelay( m.method_.digitizer_delay_to_first_sample );

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
        if ( success = task.spDriver()->setSampleRate( samp_rate ) )
            break;
    }
        
    if ( m.mode_ == 0 ) { // Digitizer 

        task.spDriver()->setAcquisitionMode( AGMD2_VAL_ACQUISITION_MODE_NORMAL );
        task.spDriver()->setAcquisitionRecordSize( m.method_.digitizer_nbr_of_s_to_acquire );
        task.spDriver()->setAcquisitionNumRecordsToAcquire( m.method_.nbr_records );

    } else { // Averager

        task.spDriver()->setAcquisitionMode( AGMD2_VAL_ACQUISITION_MODE_AVERAGER );
        task.spDriver()->setDataInversionEnabled( m.method_.invert_signal ? true : false );
        task.spDriver()->setAcquisitionRecordSize( m.method_.digitizer_nbr_of_s_to_acquire );
        task.spDriver()->setAcquisitionNumRecordsToAcquire( 1 );
        task.spDriver()->setAcquisitionNumberOfAverages( m.method_.nbr_of_averages );

    }

    ADTRACE() << "Calibrating...";
    task.spDriver()->CalibrationSelfCalibrate();

	return true;
}

bool
device::setup( task& task, const acqrscontrols::u5303a::method& m )
{
    // Originally, this was considered to use 'protocol' acquisition, which change acquisition parameter for each scan
    // However, acquisition parameter change require self-calibration on U5303A so that this can't be used.
    return true;
}

bool
device::acquire( task& task )
{
    return task.spDriver()->AcquisitionInitiate();
}

bool
device::waitForEndOfAcquisition( task& task, int timeout )
{
	(void)timeout;

    long const timeoutInMs = 3000;
    return task.spDriver()->AcquisitionWaitForAcquisitionComplete(timeoutInMs);
}

/////////////
bool
digitizer::readData( AgMD2& md2, const acqrscontrols::u5303a::method& m, std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > >& vec )
{
    ViInt64 arraySize = 0;
    const int64_t recordSize = m.method_.digitizer_nbr_of_s_to_acquire;
    const int64_t numRecords = m.method_.nbr_records;
    
    if ( AgMD2::log(
             AgMD2_QueryMinWaveformMemory( md2.session(), 16, numRecords, 0, recordSize, &arraySize )
             , __FILE__, __LINE__ ) ) {

        std::vector<ViInt16> dataArray( arraySize );
        
        ViInt64 actualRecords(0), waveformArrayActualSize(0);
        std::vector<ViInt64> actualPoints( numRecords ), firstValidPoints( numRecords );
        std::vector<ViReal64> initialXOffset( numRecords ), initialXTimeSeconds( numRecords ), initialXTimeFraction( numRecords );
        ViReal64 xIncrement(0), scaleFactor(0), scaleOffset(0);

        auto tp = std::chrono::steady_clock::now();
        
        if ( AgMD2::log( AgMD2_FetchMultiRecordWaveformInt16( md2.session()
                                                              , "Channel1"
                                                              , 0
                                                              , numRecords
                                                              , 0
                                                              , recordSize
                                                              , arraySize
                                                              , dataArray.data()
                                                              , &waveformArrayActualSize
                                                              , &actualRecords
                                                              , actualPoints.data()
                                                              , firstValidPoints.data()
                                                              , initialXOffset.data()
                                                              , initialXTimeSeconds.data()
                                                              , initialXTimeFraction.data()
                                                              , &xIncrement
                                                              , &scaleFactor
                                                              , &scaleOffset ), __FILE__, __LINE__ ) ) {

            for ( int64_t iRecord = 0; iRecord < actualRecords; ++iRecord ) {

                if ( auto data = std::make_shared< acqrscontrols::u5303a::waveform >( md2.Identify(), md2.dataSerialNumber() ) ) {

                    data->timeSinceEpoch_ = std::chrono::duration_cast<std::chrono::nanoseconds>( tp.time_since_epoch() ).count();
                    data->method_ = m;
                    data->meta_.actualAverages = 0; // digitizer
                    data->meta_.actualPoints = actualPoints[ iRecord ];
                    data->meta_.initialXTimeSeconds = initialXTimeSeconds[ iRecord ] + initialXTimeFraction[ iRecord ];
                    data->meta_.xIncrement = xIncrement;
                    data->meta_.scaleFactor = scaleFactor;
                    data->meta_.scaleOffset = scaleOffset;

                    data->resize( data->meta_.actualPoints );
            
                    std::copy( dataArray.begin() + firstValidPoints[iRecord]
                               , dataArray.begin() + firstValidPoints[iRecord] + data->meta_.actualPoints, data->begin() );

                    vec.push_back( data );
                }
            }
            return true;
        }
    }
    return false;
}

bool
digitizer::readData16( AgMD2& md2, const acqrscontrols::u5303a::method& m, acqrscontrols::u5303a::waveform& data )
{
    const int64_t recordSize = m.method_.digitizer_nbr_of_s_to_acquire;
    ViInt64 const numRecords = 1;
    ViInt64 arraySize(0);

    if ( AgMD2::log(
             AgMD2_QueryMinWaveformMemory( md2.session(), 32, 1, 0, recordSize, &arraySize )
             , __FILE__, __LINE__ ) ) {

        //vector<ViInt16> dataArray( arraySize );
        ViInt32 actualAverages(0);
        ViInt64 actualRecords(0);
        ViInt64 actualPoints[numRecords] = {0}, firstValidPoint[numRecords] = {0};
        ViReal64 initialXTimeSeconds[numRecords] = {0}, initialXTimeFraction[numRecords] = {0};
        ViReal64 initialXOffset(0), xIncrement(0), scaleFactor(0), scaleOffset(0);
        
        if ( AgMD2::log( AgMD2_FetchWaveformInt32( md2.session()
                                                   , "Channel1"
                                                   , arraySize
                                                   , reinterpret_cast<ViInt32*>(data.data( arraySize ))
                                                   , actualPoints
                                                   , firstValidPoint
                                                   , &initialXOffset
                                                   , initialXTimeSeconds
                                                   , initialXTimeFraction
                                                   , &xIncrement
                                                   , &scaleFactor
                                                   , &scaleOffset ), __FILE__, __LINE__ ) ) {

            data.timeSinceEpoch_ = std::chrono::steady_clock::now().time_since_epoch().count();
            data.method_ = m;
            data.meta_.actualAverages = actualAverages;
            data.meta_.actualPoints = actualPoints[ 0 ];
            data.meta_.initialXTimeSeconds = initialXTimeSeconds[ 0 ] + initialXTimeFraction[ 0 ];
            data.meta_.xIncrement = xIncrement;
            data.meta_.scaleFactor = scaleFactor;
            data.meta_.scaleOffset = scaleOffset;
            data.firstValidPoint_ = firstValidPoint[0];
            
            return true;
        }
    }
    return false;
}

bool
digitizer::readData32( AgMD2& md2, const acqrscontrols::u5303a::method& m, acqrscontrols::u5303a::waveform& data )
{
    ViInt64 const numRecords = 1;
    const int64_t recordSize = m.method_.digitizer_nbr_of_s_to_acquire;
    ViInt64 arraySize(0);

    if ( AgMD2::log(
             AgMD2_QueryMinWaveformMemory( md2.session(), 32, 1, 0, recordSize, &arraySize )
             , __FILE__, __LINE__ ) ) {

        data.data( arraySize );

        // vector<ViInt16> dataArray( arraySize );
        ViInt32 actualAverages(0);
        ViInt64 actualRecords(0);
        ViInt64 actualPoints[numRecords] = {0}, firstValidPoint[numRecords] = {0};
        ViReal64 initialXTimeSeconds[numRecords] = {0}, initialXTimeFraction[numRecords] = {0};
        ViReal64 initialXOffset(0), xIncrement(0), scaleFactor(0), scaleOffset(0);
        ViInt32 flags[numRecords];
        
        if ( AgMD2::log( AgMD2_FetchAccumulatedWaveformInt32( md2.session()
                                                              , "Channel1"
                                                              , 0
                                                              , 1
                                                              , 0
                                                              , recordSize
                                                              , arraySize
                                                              , reinterpret_cast<ViInt32*>(data.data( arraySize ))
                                                              , &actualAverages
                                                              , &actualRecords
                                                              , actualPoints
                                                              , firstValidPoint
                                                              , &initialXOffset
                                                              , initialXTimeSeconds
                                                              , initialXTimeFraction
                                                              , &xIncrement
                                                              , &scaleFactor
                                                              , &scaleOffset, flags ), __FILE__, __LINE__ ) ) {
            
            data.timeSinceEpoch_ = std::chrono::steady_clock::now().time_since_epoch().count();
            data.method_ = m;
            data.meta_.actualAverages = actualAverages;
            data.meta_.actualPoints = actualPoints[ 0 ];
            data.meta_.initialXTimeSeconds = initialXTimeSeconds[ 0 ] + initialXTimeFraction[ 0 ];
            data.meta_.xIncrement = xIncrement;
            data.meta_.scaleFactor = scaleFactor;
            data.meta_.scaleOffset = scaleOffset;
            data.firstValidPoint_ = firstValidPoint[0];
            
            // auto dp = data.data( numPointsPerRecord );            
            // std::copy( sa.data() + firstValidPoint, sa.data() + numPointsPerRecord, dp );            
            
            return true;
        }
    }
    return false;
}
