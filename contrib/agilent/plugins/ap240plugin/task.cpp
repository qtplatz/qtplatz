/**************************************************************************
** Copyright (C) 2014-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this 
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms 
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "task.hpp"
#include "constants.hpp"
#include "document.hpp"
#include "tdcdoc.hpp"
#include <acqrscontrols/constants.hpp>
#include <acqrscontrols/ap240/waveform.hpp>
#include <acqrscontrols/ap240/threshold_result.hpp>
#include <acqrscontrols/ap240/method.hpp>
#include <ap240/digitizer.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/mappedimage.hpp>
#include <adcontrols/mappedspectra.hpp>
#include <adcontrols/mappedspectrum.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/trace.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/debug.hpp>
#include <adportable/semaphore.hpp>
#include <adportable/binary_serializer.hpp>
#include <adicontroller/instrument.hpp>
#include <adicontroller/signalobserver.hpp>
#include <adicontroller/sampleprocessor.hpp>
#include <adlog/logger.hpp>
#include <workaround/boost/asio.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/numeric/ublas/fwd.hpp> // matrix forward decl
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/variant.hpp>
#include <QRect>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <numeric>
#include <thread>

namespace ap240 {

    static std::once_flag flag1;

    struct data_status {
        uint32_t pos_;
        uint32_t pos_origin_;
        int device_version_;
        uint32_t posted_data_count_;
        uint32_t proced_data_count_;
        std::atomic< bool > plot_ready_;
        std::atomic< bool > data_ready_;        
        std::chrono::steady_clock::time_point tp_data_handled_;
        std::chrono::steady_clock::time_point tp_plot_handled_;
        data_status() : pos_( -1 ), pos_origin_( 0 ), device_version_( 0 ), posted_data_count_( 0 ), plot_ready_( false ), data_ready_( false ) {
        }
        data_status( const data_status& t ) : pos_( t.pos_ )
                                            , pos_origin_( t.pos_origin_ )
                                            , device_version_( t.device_version_ )
                                            , posted_data_count_( t.posted_data_count_ )
                                            , proced_data_count_( t.proced_data_count_ )
                                            , tp_data_handled_( t.tp_data_handled_ )
                                            , tp_plot_handled_( t.tp_plot_handled_ ) {
        }
    };

    class task::impl {
    public:
        impl() : worker_stopping_( false )
               , work_( io_service_ )
               , traceAccessor_( std::make_shared< adcontrols::TraceAccessor >() )
               , software_inject_triggered_( false )
               , histogram_window_enabled_( false )
               , device_delay_count_( 0 ) {
            
        }

        static std::atomic< task * > instance_;
        static std::mutex mutex_;

        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;

        std::vector< std::thread > threads_;
        adportable::semaphore sema_;
        std::atomic< bool > worker_stopping_;
        std::chrono::steady_clock::time_point tp_uptime_;
        std::chrono::steady_clock::time_point tp_inject_;

        std::map< boost::uuids::uuid, data_status > data_status_;

        std::shared_ptr< adcontrols::TraceAccessor > traceAccessor_;
        std::atomic< bool > software_inject_triggered_;
        std::atomic< bool > histogram_window_enabled_;

        bool histogram_clear_cycle_enabled_;
        uint32_t histogram_clear_cycle_;
        std::condition_variable cv_;

        std::vector< std::array< std::shared_ptr< acqrscontrols::ap240::threshold_result >, 2 > > que_;

        // std::deque < std::shared_ptr< adicontroller::SampleProcessor > > acquireingSamples_;
        // std::deque < std::shared_ptr< adicontroller::SampleProcessor > > processingSamples_;
        acqrscontrols::ap240::metadata metadata_;
        uint32_t device_delay_count_;

        void worker_thread();
        bool finalize();
        void readData( adicontroller::SignalObserver::Observer *, uint32_t pos );

        void handle_ap240_data( data_status&, std::shared_ptr< adicontroller::SignalObserver::DataReadBuffer > rb );
        void handle_ap240_average( const data_status, std::array< std::shared_ptr< acqrscontrols::ap240::threshold_result >, 2 > );

        void resetDeviceData() {
            traceAccessor_->clear();
            inject_triggered();  // reset time
        }

        void clear_histogram() {
        }

        void inject_triggered() {
            tp_inject_ = std::chrono::steady_clock::now();
        }

        template<typename Rep, typename Period> Rep uptime() const {
            return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>( std::chrono::steady_clock::now() - tp_uptime_ ).count();
        }

        template<typename Rep, typename Period> Rep timeSinceInject() const {
            return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>( std::chrono::steady_clock::now() - tp_inject_ ).count();
        }
    };

    std::atomic< task * > task::impl::instance_( 0 );
    std::mutex task::impl::mutex_;
}

using namespace ap240;

task::task() : impl_( new impl() )
{
}

task::~task()
{
    finalize();  // make sure
    delete impl_;
}

bool
task::initialize()
{
    std::call_once( flag1, [=] () {
            
            impl_->threads_.push_back( adportable::asio::thread( [=] { impl_->worker_thread(); } ) );
            
            unsigned nCores = std::max( unsigned( 3 ), std::thread::hardware_concurrency() ) - 1;
            ADTRACE() << nCores << " threads created for ap240 task";
            while( nCores-- )
                impl_->threads_.push_back( adportable::asio::thread( [=] { impl_->io_service_.run(); } ) );

        } );

    return true;
}

bool
task::finalize()
{
    return impl_->finalize();
}


bool
task::impl::finalize()
{
    if ( !worker_stopping_ ) {
        
        worker_stopping_ = true;
        sema_.signal();
    
        io_service_.stop();
        for ( auto& t : threads_ )
            t.join();
    }
    
    return true;
}

void
task::instInitialize( adicontroller::Instrument::Session * session )
{
    ADDEBUG() << "===== instInitialize =====";

    auto self( session->shared_from_this() );
    if ( self ) {
        impl_->io_service_.post( [self] () { self->initialize(); } );
    }
}

void
task::onDataChanged( adicontroller::SignalObserver::Observer * so, uint32_t pos )
{
    // on SignalObserver::Observer masharing (sync with device data-reading thread)
    impl_->io_service_.post( [=]{ impl_->readData( so, pos ); } );
}

task *
task::instance()
{
    static task __instance;
    return &__instance;
}

void
task::post( std::vector< std::future<bool> >& futures )
{
    bool processed( false );
    static std::mutex m;
    static std::condition_variable cv;

    impl_->io_service_.post( [&] () {

            std::vector< std::future<bool> > xfutures;
            for ( auto& future : futures )
                xfutures.push_back( std::move( future ) );

            { std::lock_guard< std::mutex > lk( m ); processed = true; }  cv.notify_one();

            std::for_each( xfutures.begin(), xfutures.end(), [] ( std::future<bool>& f ) { f.get(); } );
        });

    std::unique_lock< std::mutex > lock( m );
    cv.wait( lock, [&processed] { return processed; } );
}

//////////////
void
task::impl::worker_thread()
{
    do {
        sema_.wait();

        if ( worker_stopping_ )
            return;

        auto& status = data_status_[ ap240_observer ];

        if ( status.plot_ready_ ) {

            status.plot_ready_ = false;
            status.tp_plot_handled_ = std::chrono::steady_clock::now();

            int channel = 0;
            std::array< std::shared_ptr< acqrscontrols::ap240::threshold_result >, 2 > threshold_results;
            do {
                std::lock_guard< std::mutex > lock( mutex_ );
                threshold_results = que_.back();
            } while ( 0 );

            for ( auto result: threshold_results ) {
                if ( result ) {
                    auto ms = std::make_shared< adcontrols::MassSpectrum >();
                    
                    if ( acqrscontrols::ap240::waveform::translate( *ms, *result ) ) {
                        
                        ms->getMSProperty().setTrigNumber( status.pos_, status.pos_origin_ );
                        document::instance()->setData( ap240_observer, ms, channel );
                        
                    }
                }
                ++channel;
            }
            
        }

        if ( status.data_ready_ ) {
            status.data_ready_ = false;
            status.tp_data_handled_ = std::chrono::steady_clock::now();
            document::instance()->commitData();
        }

    } while ( true );
}

// strand::wrap
void
task::impl::readData( adicontroller::SignalObserver::Observer * so, uint32_t pos )
{
    if ( so ) {

        auto& status = data_status_[ so->objid() ];
        status.posted_data_count_++;

        if ( status.pos_ == uint32_t( -1 ) ) {
            status.pos_ = pos;
            status.pos_origin_ = pos;
        }
    
        if ( so->objid() == ap240_observer ) {

            std::shared_ptr< adicontroller::SignalObserver::DataReadBuffer > rb;
            if ( ( rb = so->readData( status.pos_ ) ) ) {
                status.pos_++;
                handle_ap240_data( status, rb );
            } while ( rb && status.pos_ <= pos );

        } else {
            std::string name = so->objtext();
            auto uuid = so->objid();
            ADTRACE() << "Unhandled data : " << name;
        }
    }
}

void
task::impl::handle_ap240_data( data_status& status, std::shared_ptr<adicontroller::SignalObserver::DataReadBuffer> rb )
{

    auto waveforms = acqrscontrols::ap240::waveform::deserialize( rb.get() );

    auto threshold_results = document::instance()->tdc()->handle_waveforms( waveforms );
    // todo: result -> DataReadBuffer for save data

    if ( threshold_results[0] || threshold_results[1] ) {

        document::instance()->tdc()->appendHistogram( threshold_results );
        handle_ap240_average( status, threshold_results ); // draw spectrogram and TIC

    }
}

void
task::impl::handle_ap240_average( const data_status status, std::array< threshold_result_ptr, 2 > threshold_results )
{
    do {
        std::lock_guard< std::mutex > lock( mutex_ );

        que_.push_back( threshold_results );
        if ( que_.size() >= 10 )
            que_.erase( que_.begin(), que_.begin() + 5 );
        
    } while( 0 ) ;
    
    auto tp = std::chrono::steady_clock::now();

    if ( std::chrono::duration_cast<std::chrono::milliseconds> ( tp - status.tp_plot_handled_ ).count() >= 200 ) {

        data_status_[ ap240_observer ].plot_ready_ = true;
        sema_.signal();

    }
    
    if ( std::chrono::duration_cast<std::chrono::milliseconds> ( tp - status.tp_data_handled_ ).count() >= 1000 ) {
        
        data_status_[ ap240_observer ].data_ready_ = true;
        sema_.signal();

    }
    
}

void
task::prepare_next_sample( adicontroller::SignalObserver::Observer * masterObserver
                           , std::shared_ptr< adcontrols::SampleRun > srun, const adcontrols::ControlMethod::Method& cm )
{
    auto method = std::make_shared< adcontrols::ControlMethod::Method >( cm );
    if ( auto proc = std::make_shared< adicontroller::SampleProcessor >( srun, method ) ) {

        proc->prepare_storage( masterObserver );
        emit document::instance()->sampleRunChanged();

    }

}

void
task::clear_histogram()
{
    impl_->clear_histogram();
}


void
task::setHistogramClearCycleEnabled( bool enable )
{
    impl_->histogram_clear_cycle_enabled_ = enable;
}

void
task::setHistogramClearCycle( uint32_t value )
{
    impl_->histogram_clear_cycle_ = value;
}

