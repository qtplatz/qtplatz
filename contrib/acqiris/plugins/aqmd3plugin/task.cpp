/**************************************************************************
 ** Copyright (C) 2014-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "tdc.hpp"
#include "constants.hpp"
#include "document.hpp"
#include <aqmd3/constants.hpp>
#include <aqmd3controls/histogram.hpp>
#include <aqmd3controls/histogram_adder.hpp>
#include <aqmd3controls/method.hpp>
#include <aqmd3controls/pkd_result.hpp>
#include <aqmd3controls/threshold_result.hpp>
#include <aqmd3controls/waveform.hpp>
#include <aqmd3controls/waveform_adder.hpp>
#include <aqmd3/digitizer.hpp>
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
#include <adportable/counting/process_threshold.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_type.hpp>
#include <adportable/semaphore.hpp>
#include <adportable/binary_serializer.hpp>
#include <adacquire/instrument.hpp>
#include <adacquire/signalobserver.hpp>
#include <adacquire/sampleprocessor.hpp>
#include <adacquire/task.hpp>
#include <adacquire/timedigital_histogram_accessor.hpp>
#include <adlog/logger.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/asio.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/numeric/ublas/fwd.hpp> // matrix forward decl
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/variant.hpp>
#include <QRect>
#include <QJsonDocument>
#include <QJsonObject>
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

namespace aqmd3 {

    static std::once_flag flag1;

    typedef std::shared_ptr< aqmd3controls::threshold_result > threshold_result_ptr;
    typedef std::shared_ptr< const aqmd3controls::threshold_result > const_threshold_result_ptr;
    typedef aqmd3controls::waveform waveform_type;
    typedef aqmd3controls::histogram histogram_type;

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
               , refreshHistogram_( false )
               , histogram_clear_cycle_( 100 )
               , device_delay_count_( 0 ) {

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
        std::atomic< bool > refreshHistogram_;

        uint32_t histogram_clear_cycle_;
        std::condition_variable cv_;

        // std::array< std::shared_ptr< aqmd3controls::threshold_result >, 2 > que_;
        aqmd3controls::meta_data metadata_;
        uint32_t device_delay_count_;

        std::pair< std::shared_ptr< const aqmd3controls::waveform >, std::shared_ptr< const aqmd3controls::waveform > > que_;

        void worker_thread();
        bool finalize();
        void readData( adacquire::SignalObserver::Observer *, uint32_t pos );

        void handle_aqmd3_data( data_status&, std::shared_ptr< adacquire::SignalObserver::DataReadBuffer > rb );
        void handle_aqmd3_average( const data_status, std::array< threshold_result_ptr, 2 > );

        void resetDeviceData() {
            traceAccessor_->clear();
            inject_triggered();  // reset time
        }

        void clear_histogram() {
        }

        void inject_triggered() {
            tp_inject_ = std::chrono::system_clock::now();
        }

        void handle_waveforms();
        void handle_histograms();

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

using namespace aqmd3;

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
            ADTRACE() << nCores << " threads created for aqmd3 task";
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

//////////////
void
task::impl::worker_thread()
{
    //using aqmd3controls::aqmd3::tdcdoc;

    do {
        sema_.wait();

        if ( worker_stopping_ )
            return;

        // waveform
        if ( data_status_[ waveform_observer ].plot_ready_ ) {

            auto& status = data_status_[ waveform_observer ];
            status.plot_ready_ = false;
            status.tp_plot_handled_ = std::chrono::system_clock::now();
            auto q(que_); // lock;

            std::string device_data;
            using aqmd3controls::waveform;
            q.first->serialize_xmeta( device_data );

            int channel(0);
            if ( auto avg = q.first ) {
                auto ms = std::make_shared< adcontrols::MassSpectrum >();
                adcontrols::waveform_translator::translate< waveform >(
                    *ms
                    , *avg
                    , avg->xmeta().xIncrement
                    , avg->xmeta().initialXOffset
                    , avg->xmeta().actualAverages
                    , 0 // mode
                    , "adplugins.datainterpreter.ms-cheminfo.com" // see datareader_factory.cpp
                    , device_data
                    , [&]( const int32_t& d ){ return 1000 * avg->toVolts( d ); } );
                document::instance()->setData( waveform_observer, ms, channel++ );
            }

            if ( auto pkd = q.second ) {
                auto ms = std::make_shared< adcontrols::MassSpectrum >();
                aqmd3controls::waveform::translate( *ms, *pkd );
#if 0
                adcontrols::waveform_translator::translate< waveform >(
                    *ms
                    , *pkd
                    , pkd->xmeta().xIncrement
                    , pkd->xmeta().initialXOffset
                    , pkd->xmeta().actualAverages
                    , 0 // mode
                    , "adplugins.datainterpreter.ms-cheminfo.com" // see datareader_factory.cpp
                    , device_data
                    );
#endif
                document::instance()->setData( waveform_observer, ms, channel++ );
            }
        }

        if ( data_status_[ waveform_observer ].data_ready_ ) {
            data_status_[ waveform_observer ].data_ready_ = false;
            data_status_[ waveform_observer ].tp_data_handled_ = std::chrono::system_clock::now();
            io_service_.post( strand_.wrap( [&]() { document::instance()->commitData(); } ) );
        }

        // Histogram
        if ( data_status_[ histogram_observer ].plot_ready_ ) {
            auto& status = data_status_[ histogram_observer ];
            status.plot_ready_ = false;

            //if ( auto ms = document::instance()->tdc()->recentSpectrum( choice, mass_assignee ) ) {...
#if 0
            if ( auto hgrm = document::instance()->longTermHistogram( 0 ) ) {
                double resolution = 0;
                //auto tm = tdc->threshold_method();
                //resolution = tm->time_resolution;

                if ( resolution > hgrm->xIncrement() ) {
                    adcontrols::TimeDigitalHistogram::vector_type time_merged;
                    //std::vector< std::pair< double, uint32_t > > time_merged;
                    //adcontrols::TimeDigitalHistogram::average_time( hgrm->histogram(), resolution, time_merged );
                    hgrm = hgrm->merge_peaks( resolution );
                }

                auto ms = std::make_shared< adcontrols::MassSpectrum >();
                adcontrols::TimeDigitalHistogram::translate( *ms, *hgrm );
                document::instance()->setData( histogram_observer, ms, 0 );
            }
            status.tp_plot_handled_ = std::chrono::system_clock::now();
#endif
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

        if ( so->objid() == waveform_observer ) {

            if ( auto rb = so->readData( pos ) )
                handle_aqmd3_data( status, rb );

        } else {
            ADTRACE() << "Unhandled data : " << so->objtext() << ", " << so->objid();
        }
    }
}

void
task::impl::handle_aqmd3_data( data_status& status, std::shared_ptr<adacquire::SignalObserver::DataReadBuffer> rb )
{
    typedef std::pair< std::shared_ptr< const aqmd3controls::waveform >
                       , std::shared_ptr< const aqmd3controls::waveform > > const_waveform_pair_t;

    std::shared_ptr< const aqmd3controls::waveform > avg, pkd;
    // std::array< std::shared_ptr< const aqmd3controls::waveform >, 2 > waveforms( {{0, 0}} );

    if ( adportable::a_type< const_waveform_pair_t >::is_a( rb->data() ) ) {
        try {
            std::tie( avg, pkd ) = boost::any_cast< const_waveform_pair_t >( rb->data() );
        } catch ( boost::bad_any_cast& ) {
            assert( 0 );
            ADERROR() << "bad any cast";
            return;
        }
    } else if ( adportable::a_type< std::shared_ptr< const aqmd3controls::waveform > >::is_a( rb->data() ) ) {
        avg = boost::any_cast< std::shared_ptr< const aqmd3controls::waveform > >( rb->data() );
        pkd = nullptr;
    }

    using namespace std::chrono_literals;
    const auto tp = std::chrono::system_clock::now();

    // data commit on file every second
    if ( ( tp - status.tp_data_handled_ ) >= 1000ms ) {
        status.data_ready_ = true;
        sema_.signal();
    }

    if ( avg ) {
        // waveform_observer == waveform from digitizer (raw)
        if ( ( tp - data_status_[ waveform_observer ].tp_plot_handled_ ) >= 250ms ) {
            que_ = std::make_pair( avg, pkd );
            status.plot_ready_ = true; // ads54j single trigger waveform
            sema_.signal();
        }

        if ( avg->xmeta().actualAverages > 0 ) { // hard averaged waveform
            // TODO
            // document::instance()->enqueue( avg, pkd );                                        // <========== to data file
            // if ( ( tp - data_status_[ avrg_waveform_observer ].tp_plot_handled_ ) >= 250ms )
            //     io_service_.post( [this](){ handle_waveforms(); } );

            // if ( ( tp - data_status_[ histogram_observer ].tp_plot_handled_ ) >= 250ms )
            //     io_service_.post( [this](){ handle_histograms(); } );
        } else { // soft average
            // make an averaged waveform
            const int protocol(0);
            if ( auto adder = tdc::instance()->waveform_adder( protocol ) ) {
                if ( adder->add( *avg ) == 100 ) {
                    auto avgd = adder->fetch();
                    tdc::instance()->set_periodic_avgd_waveforms( protocol, avgd );
                    tdc::instance()->add_longterm_avgd_waveforms( protocol, avgd );
                    tdc::instance()->enqueue( avgd );
                    if ( !tdc::instance()->waveform_empty() && ( tp - data_status_[ avrg_waveform_observer ].tp_plot_handled_ ) >= 250ms ) {
                        io_service_.post( [this](){ this->handle_waveforms(); } );
                    }
                }
            }
            adportable::counting::process_threshold< waveform_t, aqmd3controls::pkd_result > threshold_finder;
            threshold_finder.set_slope( adportable::counting::CrossDown );
            threshold_finder.set_threshold_level( 0.010, avg->toBinary( 0.010 ) );
            if ( auto result = threshold_finder( avg ) ) {

            }

            // if ( document::instance()->accumulate_waveform( avg ) && ( tp - data_status_[ avrg_waveform_observer ].tp_plot_handled_ ) >= 250ms ) {
            //     io_service_.post( [this](){ handle_waveforms(); } );
            // }

            // if ( auto result = document::instance()->processThreshold3( avg ) ) {
            //     io_service_.post( [=] () { document::instance()->result_to_file( result ); } );  // save trigger,peaks in the data
            //     // make histogram (long-term & periodical)
            //     if ( document::instance()->accumulate_histogram( result ) && ( ( tp - data_status_[ histogram_observer ].tp_plot_handled_ ) > 250ms ) ) {
            //         io_service_.post( [this](){ handle_histograms(); } );
            //     }
            // } else {
            //     // ADDEBUG() << "##### got nullptr from processThreshold3 #####";
            // }
        }

    }


#if 0
    if ( waveforms[0] || waveforms[1] ) {
        auto threshold_results = document::instance()->tdc()->processThreshold2( waveforms );

        const auto tp = std::chrono::system_clock::now();

        if ( threshold_results[0] || threshold_results[1] ) {

            if ( threshold_results[0] )  {

                io_service_.post( [=]() { document::instance()->result_to_file( threshold_results.at(0) ); } );

                // make histogram (long-term & periodical)
                if ( document::instance()->tdc()->accumulate_histogram( threshold_results [ 0 ] ) &&
                     ( ( tp - data_status_[ histogram_observer ].tp_plot_handled_ ) > 250ms ) ) {

                    data_status_[ histogram_observer ].tp_plot_handled_ = tp;
                    io_service_.post( [this](){ handle_histograms(); } );
                }

                // single trigger waveform
                if ( ( tp - data_status_[ aqmd3_observer ].tp_plot_handled_ ) >= 250ms ) {
                    que_ = threshold_results;
                    status.plot_ready_ = true; // aqmd3
                    sema_.signal();

                }

                // data
                if ( ( tp - status.tp_data_handled_ ) >= 1000ms ) {

                    status.data_ready_ = true;  // aqmd3
                    sema_.signal();

                }

            }
        }
    }
#endif
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
task::impl::handle_waveforms()
{
}

void
task::impl::handle_histograms()
{
    auto accessor = std::make_shared< adacquire::timedigital_histogram_accessor >();
#if 0
    size_t ndata(0);
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        ndata = document::instance()->readTimeDigitalHistograms( accessor->vec );
    } while ( 0 );

    if ( ndata ) {
        // ============= write Time digital countgram to .adfs file ====================
        // do {
        //     auto tmp = std::make_shared< adacquire::SignalObserver::DataWriter >( accessor );
        //     io_service_.post( [=](){ adacquire::task::instance()->handle_write( histogram_observer, tmp ); } );
        // } while (0 );
        // <============================================================================

        // for ( auto& ptr: accessor->vec ) {
        //     std::vector< uint32_t > results; // counts
        //     //document::instance()->makeCountingChromatogramPoints( *ptr, results );
        //     //document::instance()->addCountingChromatogramsPoint( ptr->timeSinceEpoch().first, ptr->serialnumber().first, results );
        // }

        if ( auto hgrm = accessor->vec.back() ) {
            data_status_[ histogram_observer ].plot_ready_ = true;
            sema_.signal();
        }
    }
#endif
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
