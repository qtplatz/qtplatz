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
#include "mass_assignor.hpp"
#include <acqrscontrols/constants.hpp>
#include <acqrscontrols/softaveraged_waveform_accessor.hpp>
#include <acqrscontrols/u5303a/method.hpp>
#include <acqrscontrols/u5303a/tdcdoc.hpp>
#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <acqrscontrols/u5303a/waveform.hpp>
#include <adacquire/constants.hpp>
#include <adacquire/instrument.hpp>
#include <adacquire/sampleprocessor.hpp>
#include <adacquire/signalobserver.hpp>
#include <adacquire/task.hpp>
#include <adacquire/timedigital_histogram_accessor.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/trace.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adlog/logger.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_type.hpp>
#include <adportable/semaphore.hpp>
#include <u5303a/digitizer.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/numeric/ublas/fwd.hpp> // matrix forward decl
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/variant.hpp>
#include <boost/asio.hpp>
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

namespace accutof { namespace acquire {

        static std::once_flag flag1;

        typedef std::shared_ptr< acqrscontrols::u5303a::threshold_result > threshold_result_ptr;
        typedef std::shared_ptr< const acqrscontrols::u5303a::threshold_result > const_threshold_result_ptr;
        typedef acqrscontrols::u5303a::waveform waveform_type;
        typedef acqrscontrols::u5303a::histogram histogram_type;

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

        typedef std::array< std::shared_ptr< acqrscontrols::u5303a::threshold_result >, 2 > threshold_array_type;
        typedef std::array< std::shared_ptr< const acqrscontrols::u5303a::waveform >, 2 > waveform_array_type;
        typedef boost::variant< threshold_array_type, waveform_array_type > que_variant_type;

        using work_guard_type = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

        class task::impl {
        public:
            impl() : tp_uptime_( std::chrono::system_clock::now() )
                   , tp_inject_( tp_uptime_ )
                   , work_guard_( io_service_.get_executor() )
                   , strand_( io_service_ )
                   , worker_stopping_( false )
                   , traceAccessor_( std::make_shared< adcontrols::TraceAccessor >() )
                   , histogram_window_enabled_( false )
                   , histogram_clear_cycle_enabled_( false )
                   , histogram_clear_cycle_( 100 )
                   , isRecording_( true )
                   , refreshHistogram_( false )
                   , inject_triggered_( false ) {
            }

            static std::atomic< task * > instance_;
            static std::mutex mutex_;

            const std::chrono::system_clock::time_point tp_uptime_;

            std::chrono::system_clock::time_point tp_inject_;

            boost::asio::io_context io_service_;
            work_guard_type work_guard_;
            boost::asio::io_context::strand strand_;

            std::vector< std::thread > threads_;
            adportable::semaphore sema_;
            std::atomic< bool > worker_stopping_;

            std::map< boost::uuids::uuid, data_status > data_status_;

            std::shared_ptr< adcontrols::TraceAccessor > traceAccessor_;
            bool histogram_window_enabled_;
            bool histogram_clear_cycle_enabled_;
            uint32_t histogram_clear_cycle_;
            bool isRecording_;
            bool refreshHistogram_;
            bool inject_triggered_;

            std::condition_variable cv_;
            que_variant_type que_;

            acqrscontrols::u5303a::metadata metadata_;

            void worker_thread();
            bool finalize();
            void readData( adacquire::SignalObserver::Observer *, uint32_t pos );

            void handle_u5303a_data( data_status&, std::shared_ptr< adacquire::SignalObserver::DataReadBuffer > rb );

            void handle_histograms();
            void handle_averaged_waveforms();
            void handle_softaveraged_waveforms();

            void resetDeviceData() {
                traceAccessor_->clear();
                inject_triggered();  // reset time
            }

            void clear_histogram() {
            }

            void inject_triggered() {
                tp_inject_ = std::chrono::system_clock::now();
                inject_triggered_ = true;
                ADDEBUG() << "##### INJECT TRIGGERED #####";
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
}


using namespace accutof::acquire;

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
            ADTRACE() << nCores << " threads created for u5303a task";
            while( nCores-- ) {
                impl_->threads_.emplace_back( adportable::asio::thread( [=] { impl_->io_service_.run(); } ) );
            }

            adacquire::task::instance()->connect_inst_events( [&]( adacquire::Instrument::eInstEvent ev ){
                    // handle UDP port 7125 event
                    if ( ev == adacquire::Instrument::instEventInjectOut )
                        document::instance()->actionInject();
                });

            adacquire::task::instance()->initialize();

        } );

    return true;
}


bool
task::finalize()
{
    adacquire::task::instance()->finalize();
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

    return true;
}

void
task::instInitialize( adacquire::Instrument::Session * session )
{
    auto self( session->shared_from_this() );
    if ( self ) {
        boost::asio::post( impl_->io_service_, [self] () { self->initialize(); } );
        // impl_->io_service_.post( [self] () { self->initialize(); } );
    }
}

void
task::onDataChanged( adacquire::SignalObserver::Observer * so, uint32_t pos )
{
    // This thread is marshaled from SignalObserver::Observer, which is the device's data read thread

    if ( impl_->isRecording_ ) {

        impl_->data_status_[ so->objid() ].posted_data_count_++;

        boost::asio::post( impl_->io_service_, [=]{ impl_->readData( so, pos ); } );
        //impl_->io_service_.post( [=]{ impl_->readData( so, pos ); } );

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

    boost::asio::post( impl_->io_service_, [&]() {
    // impl_->io_service_.post( [&] () {

            std::vector< std::future<bool> > xfutures;
            for ( auto& future : futures )
                xfutures.emplace_back( std::move( future ) );

            { std::lock_guard< std::mutex > lk( m ); processed = true; }  cv.notify_one();

            std::for_each( xfutures.begin(), xfutures.end(), [] ( std::future<bool>& f ) { f.get(); } );
        });

    std::unique_lock< std::mutex > lock( m );
    cv.wait( lock, [&processed] { return processed; } );
}

//////////////

namespace accutof { namespace acquire {

        struct queue_process : boost::static_visitor< void > {
            data_status& status;
            queue_process ( data_status& t ) : status( t )
                {}

            // software threshold counting
            void operator()( threshold_array_type threshold_results ) const {
                int channel = 0;
                for ( auto result: threshold_results ) {
                    if ( result ) {
                        auto ms = std::make_shared< adcontrols::MassSpectrum >();
                        mass_assignor assignor;
                        if ( acqrscontrols::u5303a::waveform::translate( *ms, *result, assignor ) ) {
                            ms->getMSProperty().setTrigNumber( result->data()->serialnumber_, status.pos_origin_ );
                            document::instance()->setData( acqrscontrols::u5303a::waveform_observer, ms, channel );
                        }
                    }
                    ++channel;
                }
            }

            // pkd+avg waveforms
            void operator()( waveform_array_type wforms ) const {
                int channel = 0;

                for ( auto wform: wforms ) {
                    if ( wform ) {
                        auto ms = std::make_shared< adcontrols::MassSpectrum >();
                        if ( acqrscontrols::u5303a::waveform::translate( *ms, *wform, mass_assignor() ) ) {
                            ms->getMSProperty().setTrigNumber( wform->serialnumber_, status.pos_origin_ );
                            document::instance()->setData( acqrscontrols::u5303a::waveform_observer, ms, channel );
                        }
                    }
                    ++channel;
                }
            }
        };
    }
}

void
task::impl::worker_thread()
{
    using acqrscontrols::u5303a::tdcdoc;

    do {
        sema_.wait();

        if ( worker_stopping_ )
            return;

        // per trigger waveform
        if ( data_status_[ acqrscontrols::u5303a::waveform_observer ].plot_ready_ ) {

            auto& status = data_status_[ acqrscontrols::u5303a::waveform_observer ];

            status.plot_ready_ = false;
            status.tp_plot_handled_ = std::chrono::system_clock::now();

            boost::apply_visitor( queue_process( status ), que_ );

            if ( ! histogram_clear_cycle_enabled_ ) { // draw co-added pkd waveform
                if ( auto ms = document::instance()->tdc()->recentSpectrum( acqrscontrols::tdcbase::LongTermPkdWaveform, mass_assignor() ) )
                    document::instance()->setData( acqrscontrols::u5303a::pkd_coadd_spectrum, ms, 1 );
            }
        }

        if ( data_status_[ acqrscontrols::u5303a::waveform_observer ].data_ready_ ) {
            data_status_[ acqrscontrols::u5303a::waveform_observer ].data_ready_ = false;
            data_status_[ acqrscontrols::u5303a::waveform_observer ].tp_data_handled_ = std::chrono::system_clock::now();

            boost::asio::post( strand_, [&]() { document::instance()->commitData(); } );
        }

        // Histogram
        if ( data_status_[ acqrscontrols::u5303a::histogram_observer ].plot_ready_ ) {
            auto& status = data_status_[ acqrscontrols::u5303a::histogram_observer ];
            status.plot_ready_ = false;

            //if ( auto ms = document::instance()->tdc()->recentSpectrum( choice, mass_assignee ) ) {...
            //auto tdc = document::tdc();
            if ( auto hgrm = document::tdc()->longTermHistogram() ) {
                double resolution = 0;
                if ( auto tm = document::tdc()->threshold_method( /* ch(0|1) */ 0 ) )
                    resolution = tm->time_resolution;

                if ( resolution > hgrm->xIncrement() ) {
                    adcontrols::TimeDigitalHistogram::vector_type time_merged;
                    //std::vector< std::pair< double, uint32_t > > time_merged;
                    //adcontrols::TimeDigitalHistogram::average_time( hgrm->histogram(), resolution, time_merged );
                    hgrm = hgrm->merge_peaks( resolution );
                }

                auto ms = std::make_shared< adcontrols::MassSpectrum >();
                adcontrols::TimeDigitalHistogram::translate( *ms, *hgrm );
                document::instance()->setData( acqrscontrols::u5303a::histogram_observer, ms, 0 );
            }
            status.tp_plot_handled_ = std::chrono::system_clock::now();
        }

    } while ( true );
}

void
task::impl::readData( adacquire::SignalObserver::Observer * so, uint32_t pos )
{
    if ( so ) {

        auto& status = data_status_[ so->objid() ];

        if ( status.pos_origin_ == uint32_t( -1 ) ) {
            status.pos_origin_ = pos;
        }

        if ( so->objid() == acqrscontrols::u5303a::waveform_observer ) {

            if ( auto rb = so->readData( pos ) )
                handle_u5303a_data( status, rb );

        } else {
            std::string name = so->objtext();
            auto uuid = so->objid();
            ADTRACE() << "Unhandled data : " << name << ", " << uuid;
        }
    }
}

// copy from infitof2 (handle_u5303a_softaveraged_waveforms)
void
task::impl::handle_softaveraged_waveforms()
{
    size_t ndata(0);
    auto accessor = std::make_shared< acqrscontrols::softaveraged_waveform_accessor >();
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        ndata = document::tdc()->readAveragedWaveforms( accessor->avgd );
    } while ( 0 );

    if ( ndata ) {
        // ================== write averaged waveform to datafile ======================
        do {
            if ( auto tmp = std::make_shared< adacquire::SignalObserver::DataWriter >( accessor ) )
                adacquire::task::instance()->handle_write( acqrscontrols::u5303a::softavgr_observer, std::move( tmp ) );
            // boost::asio::post( strand_
            //                    , [tmp](){ adacquire::task::instance()->handle_write( acqrscontrols::u5303a::softavgr_observer
            // io_service_.post( strand_.wrap ( [=](){ adacquire::task::instance()->handle_write( acqrscontrols::u5303a::softavgr_observer, tmp ); } ) );
        } while (0 );
        // <============================================================================
    }
}

void
task::impl::handle_u5303a_data( data_status& status, std::shared_ptr<adacquire::SignalObserver::DataReadBuffer> rb )
{
    typedef std::pair< std::shared_ptr< const acqrscontrols::u5303a::waveform >
                       , std::shared_ptr< const acqrscontrols::u5303a::waveform > > const_waveform_pair_t;

    std::array< std::shared_ptr< const acqrscontrols::u5303a::waveform >, 2 > waveforms( {{ 0, 0 }} );

    if ( adportable::a_type< const_waveform_pair_t >::is_a( rb->data() ) ) {
        try {
            const_waveform_pair_t pair = boost::any_cast< const_waveform_pair_t >( rb->data() );
            waveforms = {{ pair.first, pair.second }};
        } catch ( boost::bad_any_cast& ) {
            ADERROR() << "bad any cast";
        }
    }

    using namespace std::chrono_literals;
    const auto tp = std::chrono::system_clock::now();

    // data commit on file every second
    if ( ( tp - status.tp_data_handled_ ) >= 1000ms ) {
        status.data_ready_ = true;  // u5303a
        sema_.signal();
    }

    if ( waveforms[ 0 ] && waveforms[ 0 ]->meta_.actualAverages > 0 ) { // Average mode data

        /////////////////////////////////////
        // ADDEBUG() << "wellKnownEvents: " << waveforms[0]->wellKnownEvents_ << " events: " << rb->events();
        /////////////////////////////////////

        // -----> on trial 28-MAR-2020
        //io_service_.post( [=]() { document::instance()->waveforms_to_file( waveforms ); } ); // append to pkdavgwriter
        document::instance()->waveforms_to_file( waveforms ); // on trial 28-MAR-2020; append to pkdavgwriter
        // <----

        // PKD+AVG (PKD)
        if ( waveforms[ 1 ] && waveforms[ 1 ]->meta_.channelMode == acqrscontrols::u5303a::PKD ) { // isPKD ?
            // compute count rate (result put in tdcdoc class)
            document::instance()->tdc()->processPKD( waveforms[ 1 ] );
        } else {
            // ADDEBUG() << "AVERAGE DATA";
        }

        auto dup( waveforms );
        auto size = document::instance()->enqueue( std::move( waveforms ) );  (void)(size);
        if ( ( tp - status.tp_plot_handled_ ) > 250ms ) {
            que_ = dup;
            status.plot_ready_ = true; // u5303a
            sema_.signal();
            // -----> on trial 28-MAR-2020
            //io_service_.post( [this]{ handle_averaged_waveforms(); } ); // <- chromatogram points calculation
            handle_averaged_waveforms();  // on trial 28-MAR-2020
            // <-----
        }

    } else if ( waveforms[0] || waveforms[1] ) {  // Threshold counting

        constexpr auto histogram_observer = acqrscontrols::u5303a::histogram_observer;
        constexpr auto softavgr_observer = acqrscontrols::u5303a::softavgr_observer;

        //-- copy from infitof2
        auto threshold_results = document::tdc()->processThreshold3( waveforms ); // processThreshold2

        // generate chromatograms & soft-averaged waveforms
        if ( auto result = threshold_results.at( 0 ) ) {

            // write all thresholds to file (puth into que)

            // -----> on trial 28-MAR-2020
            //io_service_.post( [=] () { document::instance()->result_to_file( result ); } );
            document::instance()->result_to_file( result );
            // <-----

            // make histogram (long-term & periodical)
            if ( document::tdc()->accumulate_histogram( result ) &&
                 ( ( tp - data_status_[ histogram_observer ].tp_plot_handled_ ) > 250ms ) ) {

                data_status_[ histogram_observer ].tp_plot_handled_ = tp;
                boost::asio::post( io_service_, [this](){ handle_histograms(); } );
                //io_service_.post( [this](){ handle_histograms(); } );
            }

            // (original code) single trigger waveform
            if ( ( tp - data_status_[ acqrscontrols::u5303a::waveform_observer ].tp_plot_handled_ ) >= 250ms ) {
                que_ = threshold_results;
                status.plot_ready_ = true; // u5303a
                sema_.signal();
            }

            // software average for waveform
            if ( document::tdc()->accumulate_waveform( result->data() ) &&
                 ( ( tp - data_status_[ softavgr_observer ].tp_plot_handled_ ) > 250ms ) ) {

                data_status_[ softavgr_observer ].tp_plot_handled_ = tp;
                boost::asio::post( io_service_,  [this](){ handle_softaveraged_waveforms(); } );
                //io_service_.post( [this](){ handle_softaveraged_waveforms(); } );
            }

        }
    }
}

void
task::sample_started()
{
    ADDEBUG() << "=====> sample_started()";
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
    impl_->refreshHistogram_ = m.refreshHistogram();
}

void
task::impl::handle_averaged_waveforms()
{
    if ( auto cm = document::tdc()->tofChromatogramsMethod() ) {
        std::vector< pkdavg_waveforms_t > vec;
        if ( document::instance()->dequeue( vec ) > 0 ) {
            for ( auto& waveforms: vec )
                document::instance()->addChromatogramsPoint( *cm, waveforms );
        }
    }
}

void
task::impl::handle_histograms()
{
    if ( auto tdc = document::tdc() ) {

        auto accessor = std::make_shared< adacquire::timedigital_histogram_accessor >();

        size_t ndata(0);
        do {
            std::lock_guard< std::mutex > lock( mutex_ );
            ndata = tdc->readTimeDigitalHistograms( accessor->vec );
        } while ( 0 );

        if ( ndata ) {
            // ================== write Time digital countgram to datafile ======================
            do {
                if ( auto tmp = std::make_shared< adacquire::SignalObserver::DataWriter >( accessor ) )
                    adacquire::task::instance()->handle_write( acqrscontrols::u5303a::histogram_observer, std::move( tmp ) );
                // boost::asio::post( io_service_
                //                    , [=](){ adacquire::task::instance()->handle_write( acqrscontrols::u5303a::histogram_observer, std::move( tmp ) ); } );
                //io_service_.post( [=](){ adacquire::task::instance()->handle_write( acqrscontrols::u5303a::histogram_observer, tmp ); } );
            } while (0 );
            // <============================================================================

            if ( auto cm = tdc->tofChromatogramsMethod() )
                document::instance()->addCountingChromatogramPoints( *cm, accessor->vec );

            if ( auto hgrm = accessor->vec.back() ) {
                data_status_[ acqrscontrols::u5303a::histogram_observer ].plot_ready_ = true;
                sema_.signal();
            }
        }
    }
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
