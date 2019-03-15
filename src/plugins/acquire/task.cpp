/**************************************************************************
 ** Copyright (C) 2014-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/mappedimage.hpp>
#include <adcontrols/mappedspectra.hpp>
#include <adcontrols/mappedspectrum.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/trace.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adcontrols/waveform_translator.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_type.hpp>
#include <adportable/semaphore.hpp>
#include <adportable/binary_serializer.hpp>
#include <adacquire/instrument.hpp>
#include <adacquire/signalobserver.hpp>
#include <adacquire/sampleprocessor.hpp>
#include <adacquire/samplesequence.hpp>
#include <adacquire/task.hpp>
#include <adacquire/timedigital_histogram_accessor.hpp>
#include <adacquire/waveform_accessor.hpp>
#include <adportable/date_string.hpp>
#include <adlog/logger.hpp>
#include <boost/asio.hpp>
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/numeric/ublas/fwd.hpp> // matrix forward decl
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
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

namespace acquire {

    static std::once_flag flag1;

    struct data_status {
        uint32_t pos_origin_;
        int device_version_;
        uint32_t posted_data_count_;
        uint32_t proced_data_count_;
        std::atomic< bool > plot_ready_;
        std::atomic< bool > data_ready_;
        std::chrono::system_clock::time_point tp_data_handled_;
        std::chrono::system_clock::time_point tp_plot_handled_;
        data_status() : pos_origin_( -1 )
                      , device_version_( 0 )
                      , posted_data_count_( 0 )
                      , plot_ready_( false )
                      , data_ready_( false ) {
        }
        data_status( const data_status& t ) : pos_origin_( t.pos_origin_ )
                                            , device_version_( t.device_version_ )
                                            , posted_data_count_( t.posted_data_count_ )
                                            , proced_data_count_( t.proced_data_count_ )
                                            , tp_data_handled_( t.tp_data_handled_ )
                                            , tp_plot_handled_( t.tp_plot_handled_ ) {
        }
    };

    class task::impl {
    public:
        impl() : tp_uptime_( std::chrono::system_clock::now() )
               , work_( io_service_ )
               , strand_( io_service_ )
               , worker_stopping_( false )
               , traceAccessor_( std::make_shared< adcontrols::TraceAccessor >() )
               , software_inject_triggered_( false )
               , histogram_window_enabled_( false )
               , cell_selection_enabled_( false )
               , histogram_clear_cycle_enabled_( false )
               , isRecording_( true )
               , histogram_clear_cycle_( 100 )
               , device_delay_count_( 0 )
               , timer_( io_service_ )
               , interval_( 1000 ) {
        }

        static std::atomic< task * > instance_;
        static std::mutex mutex_;

        const std::chrono::system_clock::time_point tp_uptime_;
        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
        boost::asio::io_service::strand strand_;

        std::vector< std::thread > threads_;
        adportable::semaphore sema_;
        std::atomic< bool > worker_stopping_;
        std::chrono::system_clock::time_point tp_inject_;

        std::map< boost::uuids::uuid, data_status > data_status_;

        std::shared_ptr< adcontrols::TraceAccessor > traceAccessor_;
        std::atomic< bool > software_inject_triggered_;
        std::atomic< bool > histogram_window_enabled_;
        std::atomic< bool > cell_selection_enabled_;
        std::atomic< bool > histogram_clear_cycle_enabled_;
        std::atomic< bool > isRecording_;

        uint32_t histogram_clear_cycle_;
        std::condition_variable cv_;

        uint32_t device_delay_count_;

        // for polling acquire  (use adacquire::task::timer for time event implementation)
        boost::asio::basic_waitable_timer< std::chrono::system_clock > timer_;
        std::chrono::system_clock::time_point tp_;
        uint32_t interval_;
        bool polling_enable_;

        //------------
        //----

        void worker_thread();
        bool finalize();
        void readData( adacquire::SignalObserver::Observer *, uint32_t pos );

        void handle_acquire_data( data_status&, std::shared_ptr< adacquire::SignalObserver::DataReadBuffer > rb );

        void handle_histograms();
        void handle_waveforms();

        void start_timer( uint32_t interval ) { // ms
            using namespace std::chrono_literals;

            interval_ = interval;
            auto tp = std::chrono::system_clock::now();
            auto tp0 = std::chrono::time_point_cast< std::chrono::seconds >( tp ) + 1s;
            tp_ = std::chrono::time_point_cast< std::chrono::milliseconds >( tp0 );

            timer_.expires_at( tp_ );
            timer_.async_wait( std::bind( &impl::handle_timer, this, std::placeholders::_1 ) );
        }

        void stop_timer() {
            //ADDEBUG() << "cancel timer";
            timer_.cancel();
            polling_enable_ = false;
        }

        void handle_timer( const boost::system::error_code& ec ) {
            if ( !ec ) {
                //auto prev(tp_);
                tp_ += std::chrono::milliseconds( interval_ );
                timer_.expires_at( tp_ );
                timer_.async_wait( std::bind( &impl::handle_timer, this, std::placeholders::_1 ) );
                io_service_.post( strand_.wrap( [&]() { document::instance()->poll(); } ) );
            } else {
                //ADDEBUG() << "handle_timer error";
            }
        }

        void resetDeviceData() {
            traceAccessor_->clear();
            inject_triggered();  // reset time
        }

        void inject_triggered() {
            tp_inject_ = std::chrono::system_clock::now();
        }

        template<typename Rep, typename Period> Rep uptime() const {
            return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>( std::chrono::system_clock::now() - tp_uptime_ ).count();
        }

        template<typename Rep, typename Period> Rep timeSinceInject() const {
            return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>( std::chrono::system_clock::now() - tp_inject_ ).count();
        }
    };

    std::atomic< task * > task::impl::instance_( 0 );
    std::mutex task::impl::mutex_;
}

using namespace acquire;

task::task() : impl_( new impl() )
{
}

task::~task()
{
    delete impl_;
}

bool
task::initialize()
{
    std::call_once( flag1, [=] () {

            impl_->threads_.push_back( adportable::asio::thread( [=] { impl_->worker_thread(); } ) );

            unsigned nCores = std::max( unsigned( 3 ), std::thread::hardware_concurrency() ) - 1;
            ADTRACE() << nCores << " threads created for acquire task";
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
    worker_stopping_ = true;
    sema_.signal();

    io_service_.stop();
    for ( auto& t : threads_ )
        t.join();

    ADDEBUG() << "###################### task finalize completed ################";

    return true;
}

void
task::instInitialize( adacquire::Instrument::Session * session )
{
    auto self( session->shared_from_this() );
    if ( self ) {
        impl_->io_service_.post( [self] () { self->initialize(); } );
    }
}

void
task::onDataChanged( adacquire::SignalObserver::Observer * so, uint32_t pos )
{
    // This thread is marshaled from SignalObserver::Observer, which is the device's data read thread

    ADDEBUG() << "onDataChanged: " << so->objtext() << ", pos: " << pos;

    if ( impl_->isRecording_ ) {

        impl_->data_status_[ so->objid() ].posted_data_count_++;

        impl_->io_service_.post( [=]{ impl_->readData( so, pos ); } );

    }
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

void
task::impl::handle_histograms()
{
    if ( auto doc = document::instance() ) {

        auto accessor = std::make_shared< adacquire::timedigital_histogram_accessor >();

        size_t ndata(0);
        do {
            std::lock_guard< std::mutex > lock( mutex_ );
            //ndata = doc->readTimeDigitalHistograms( accessor->vec );
        } while ( 0 );

        // ADDEBUG() << __FUNCTION__ << " ndata: " << ndata;

        if ( ndata ) {
            // ================== write Time digital countgram to datafile ======================
            // do {
            //     auto tmp = std::make_shared< adacquire::SignalObserver::DataWriter >( accessor );
            //     io_service_.post( [=](){ adacquire::task::instance()->handle_write( histogram_observer, tmp ); } );
            // } while (0 );
            // ============================================================================

            //if ( auto cm = tdc->tofChromatogramsMethod() )
            //    document::instance()->addCountingChromatogramPoints( *cm, accessor->vec );

            //if ( auto hgrm = accessor->vec.back() ) {
            //data_status_[ histogram_observer ].plot_ready_ = true;
            sema_.signal();
            //}
        }
    }

}

void
task::impl::handle_waveforms()
{
}

//////////////
void
task::impl::worker_thread()
{
    do {
        sema_.wait();

        if ( worker_stopping_ )
            return;
#if 0
        // per trigger waveform
        if ( data_status_[ WaveformObserver::__objid__ ].plot_ready_ ) {
            auto& status = data_status_[ WaveformObserver::__objid__ ];
            status.plot_ready_ = false;
            status.tp_plot_handled_ = std::chrono::system_clock::now();
            auto avgms = std::make_shared< adcontrols::MassSpectrum >();
            auto pkdms = std::make_shared< adcontrols::MassSpectrum >();
            auto q(que_); // lock;
            auto avg = q.first;
            auto pkd = q.second;

            adcontrols::waveform_translator::translate< waveform >( *avgms
                                                                    , *avg
                                                                    , avg->xIncrement()
                                                                    , avg->trigger_delay()
                                                                    , avg->xmeta().actual_averages_
                                                                    , 0 // mode
                                                                    , "acquire.dataInterpreterClsid"
                                                                    , std::string()
                                                                    , [&]( const int32_t& d ){ return 1000 * waveform::toVolts( d, avg->xmeta().actual_averages_ ); } );
            document::instance()->setData( WaveformObserver::__objid__, avgms, 0 );

            adcontrols::waveform_translator::translate< waveform >( *pkdms
                                                                    , *pkd
                                                                    , pkd->xIncrement()
                                                                    , pkd->trigger_delay()
                                                                    , pkd->xmeta().actual_averages_
                                                                    , 0 // mode
                                                                    , "acquire.dataInterpreterClsid"
                                                                    , std::string()
                                                                    , []( const int32_t& d ){ return d; } );
            pkdms->setCentroid( adcontrols::CentroidHistogram );
            document::instance()->setData( pkd_observer, pkdms, 0 );
        }

        // software averaged waveform
        if ( data_status_[ avrg_waveform_observer ].plot_ready_ ) {
            auto& status = data_status_[ avrg_waveform_observer ];
            status.plot_ready_ = false;
            status.tp_plot_handled_ = std::chrono::system_clock::now();
            auto choice = histogram_clear_cycle_enabled_ ? ProfileAvgd : ProfileLongTerm;
            if ( auto ms = document::instance()->recentSpectrum( choice ) )
                document::instance()->setData( avrg_waveform_observer, ms, 0 );
        }

        // counting data
        if ( data_status_[ histogram_observer ].plot_ready_ ) {
            auto& status = data_status_[ histogram_observer ];
            status.plot_ready_ = false;
            status.tp_plot_handled_ = std::chrono::system_clock::now();
            if ( auto ms = document::instance()->recentSpectrum( HistogramLongTerm ) ) {
                document::instance()->setData( histogram_observer, ms, 0 );
            }
        }

        // single trigger waveform
        if ( data_status_[ WaveformObserver::__objid__ ].data_ready_ ) {
            data_status_[ WaveformObserver::__objid__ ].data_ready_ = false;
            data_status_[ WaveformObserver::__objid__ ].tp_data_handled_ = std::chrono::system_clock::now();
            io_service_.post( strand_.wrap( [&]() { document::instance()->commitData(); } ) );
        }
#endif
    } while ( true );
}

void
task::impl::readData( adacquire::SignalObserver::Observer * so, uint32_t pos )
{
    // ADDEBUG() << "##### " << __FUNCTION__ << " pos: " << pos << " objid: " << so->objid() << " #####";

    if ( so ) {

        auto& status = data_status_[ so->objid() ];

        if ( status.pos_origin_ == uint32_t( -1 ) ) {
            status.pos_origin_ = pos;
        }

        //if ( so->objid() == WaveformObserver::__objid__ ) {
        //    if ( auto rb = so->readData( pos ) )
        //        handle_acquire_data( status, rb );
        //} else {
            std::string name = so->objtext();
            auto uuid = so->objid();
            ADTRACE() << "Unhandled data : " << name << " uuid: " << uuid;
            //}
    }
}

void
task::impl::handle_acquire_data( data_status& status, std::shared_ptr<adacquire::SignalObserver::DataReadBuffer> rb )
{
    // std::shared_ptr< const acquire::waveform > avg, pkd;

    // if ( adportable::a_type< const_waveform_pair_t >::is_a( rb->data() ) ) {
    //     try {
    //         std::tie( avg, pkd ) = boost::any_cast< const_waveform_pair_t >( rb->data() );
    //     } catch ( boost::bad_any_cast& ) {
    //         assert( 0 );
    //         ADERROR() << "bad any cast";
    //     }
    // } else if ( adportable::a_type< std::shared_ptr< const acquire::waveform > >::is_a( rb->data() ) ) {
    //     avg = boost::any_cast< std::shared_ptr< const acquire::waveform > >(rb->data());
    //     pkd = nullptr;
    // }

    // const auto tp = std::chrono::system_clock::now();
    // using namespace std::chrono_literals;

    // // data commit on file every second
    // if ( ( tp - status.tp_data_handled_ ) >= 1000ms ) {
    //     status.data_ready_ = true;  // acquire
    //     sema_.signal();
    // }

    // if ( avg ) {
        // waveform_observer == waveform from digitizer (raw)
        // if ( ( tp - data_status_[ WaveformObserver::__objid__ ].tp_plot_handled_ ) >= 250ms ) {
        //     que_ = std::make_pair( avg, pkd );
        //     status.plot_ready_ = true; // acquire single trigger waveform
        //     sema_.signal();
        // }

        // if ( avg->xmeta().actual_averages_ > 0 ) { // hard averaged
        //     document::instance()->enqueue( avg, pkd );
        //     if ( ( tp - data_status_[ avrg_waveform_observer ].tp_plot_handled_ ) >= 250ms )
        //         io_service_.post( [this](){ handle_waveforms(); } );

        //     if ( ( tp - data_status_[ histogram_observer ].tp_plot_handled_ ) >= 250ms )
        //         io_service_.post( [this](){ handle_histograms(); } );

        // } else { // soft average
        //     // make an averaged waveform
        //     if ( document::instance()->accumulate_waveform( avg ) && ( tp - data_status_[ avrg_waveform_observer ].tp_plot_handled_ ) >= 250ms ) {
        //         io_service_.post( [this](){ handle_waveforms(); } );
        //     }

        //     if ( auto result = document::instance()->processThreshold3( avg ) ) {
        //         io_service_.post( [=] () { document::instance()->result_to_file( result ); } );  // save trigger,peaks in the data
        //         // make histogram (long-term & periodical)
        //         if ( document::instance()->accumulate_histogram( result ) && ( ( tp - data_status_[ histogram_observer ].tp_plot_handled_ ) > 250ms ) ) {
        //             io_service_.post( [this](){ handle_histograms(); } );
        //         }
        //     } else {
        //         ADDEBUG() << "##### got nullptr from processThreshold3 #####";
        //     }
        // }
//    }
}

void
task::sample_started()
{
    ADDEBUG() << "=====> sample_started()";
    //workaround::method::instance().reset();
    //exec_method::instance().reset();
}

void
task::sample_stopped()
{
    ADDEBUG() << "=====> sample_stopped()";
    if ( auto sampleProcessor = adacquire::task::instance()->deque() ) { // ::task::instance()->sampleSequence()->deque() ) {
        // todo: post process
        ADDEBUG() << "=====> sample_stopped: " << sampleProcessor->sampleRun()->filePrefix();
    }
}

void
task::sample_injected()
{
    // invoked by exec_fsm_injected
    impl_->inject_triggered();
    //singleton::instance()->set_inject_trigger_in( true ); // set INJECT to well-known-events field on the hardware level
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

void
task::setRecording( bool rec )
{
    impl_->isRecording_ = rec;
}

bool
task::isRecording() const
{
    return impl_->isRecording_;
}

void
task::setTofChromatogramsMethod( const adcontrols::TofChromatogramsMethod& m )
{
    //impl_->refreshHistogram_ = m.refreshHistogram();
}

uint64_t
task::injectTimeSinceEpoch() const
{
    using std::chrono::nanoseconds;
    using adacquire::task;
    return std::chrono::duration_cast< nanoseconds >( task::instance()->tp_inject().time_since_epoch() ).count();
}

uint64_t
task::upTimeSinceEpoch() const
{
    return std::chrono::duration_cast< std::chrono::nanoseconds >( impl_->tp_uptime_.time_since_epoch() ).count();
}

void
task::start_polling( uint32_t interval )
{
    // // impl_->start_timer( interval );
    // impl_->io_service_.post( impl_->strand_.wrap( [&]() {
    //                                                   if ( document::instance()->poll() )
    //                                                       start_polling( 0 );
    //                                               } ) );
}
