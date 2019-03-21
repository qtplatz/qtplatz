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
#include <date/date.h>
#include <socfpga/advalue.hpp>
#include <socfpga/traceobserver.hpp>
#include <socfpga/data_accessor.hpp>
#include <boost/asio.hpp>
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/format.hpp>
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
#include <bitset>
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
        std::chrono::steady_clock::time_point tp_data_handled_;
        std::chrono::steady_clock::time_point tp_plot_handled_;
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

        void handle_trace_data( data_status&, std::shared_ptr< adacquire::SignalObserver::DataReadBuffer > rb );

        void resetDeviceData() {
            traceAccessor_->clear();
            inject_triggered();  // reset time
        }

        void inject_triggered() {
            tp_inject_ = std::chrono::system_clock::now();
        }

        void handle_inst_event( adacquire::Instrument::eInstEvent ev ) {
            if ( ev == adacquire::Instrument::instEventInjectOut )
                document::instance()->actionInject();
        }

        void handle_periodic_timer( double elapsed_time ) {
            static double last_handled = 0;

            if ( std::abs( elapsed_time - last_handled ) > 2.0 ) {
                if ( auto sample = adacquire::task::instance()->sampleSequence()->at( 0 ) ) {
                    document::instance()->progress( elapsed_time, sample->sampleRun() );
                } else {
                    ADDEBUG() << "handle timer: " << elapsed_time;
                }
                last_handled = elapsed_time;
            }
        }

         void handle_time_event( std::shared_ptr< const adcontrols::ControlMethod::TimedEvents > tt
                                 , adcontrols::ControlMethod::const_time_event_iterator begin
                                 , adcontrols::ControlMethod::const_time_event_iterator end ) {
             document::instance()->applyTimedEvent( tt, begin, end );
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
    std::call_once(
        flag1
        , [=] () {
              impl_->threads_.push_back( adportable::asio::thread( [=] { impl_->worker_thread(); } ) );

              unsigned nCores = std::max( unsigned( 3 ), std::thread::hardware_concurrency() ) - 1;
              ADTRACE() << nCores << " threads created for acquire task";
              while( nCores-- )
                  impl_->threads_.push_back( adportable::asio::thread( [=] { impl_->io_service_.run(); } ) );

              adacquire::task::instance()->connect_inst_events(
                  []( adacquire::Instrument::eInstEvent ev ){ // handle {UDP port 7125|hardware injection via signalObserver}
                      ADTRACE() << "Event: " << ev << " handled";
                      if ( ev == adacquire::Instrument::instEventInjectOut )
                          document::instance()->actionInject();
                  });

              // Listen 'inject' trigger from UDP Event Source
              adacquire::task::instance()->connect_inst_events( std::bind( &task::impl::handle_inst_event, impl_, std::placeholders::_1 ) );

              // periodic timer (100ms interval)
              adacquire::task::instance()->connect_periodic_timer( std::bind( &task::impl::handle_periodic_timer, impl_, std::placeholders::_1 ) );

              // time event handler
              adacquire::task::instance()->register_time_event_handler( std::bind( &task::impl::handle_time_event
                                                                                   , impl_
                                                                                   , std::placeholders::_1
                                                                                   , std::placeholders::_2
                                                                                   , std::placeholders::_3 ) );
              // Initialize core
              adacquire::task::instance()->initialize();
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
    if ( so ) {

        auto& status = data_status_[ so->objid() ];

        if ( status.pos_origin_ == uint32_t( -1 ) ) {
            status.pos_origin_ = pos;
        }

        if ( so->objid() == socfpga::dgmod::TraceObserver::__objid__ ) { // dgmod's adc reader
            if ( auto rb = so->readData( pos ) )
                handle_trace_data( status, rb );
        } else {
            ADTRACE() << "Unhandled data : " << so->objtext() << ", uuid: " << so->objid();
        }
    }
}

void
task::impl::handle_trace_data( data_status& status, std::shared_ptr<adacquire::SignalObserver::DataReadBuffer> rb )
{
    std::shared_ptr< std::vector< socfpga::dgmod::advalue > > data;
    if ( adportable::a_type< std::shared_ptr< std::vector< socfpga::dgmod::advalue > > >::is_a( rb->data() ) ) {
        try {
            data = boost::any_cast< std::shared_ptr< std::vector< socfpga::dgmod::advalue > > >( rb->data() );
        } catch ( boost::bad_any_cast& ex ) {
            ADDEBUG() << "bad_any_cast: " << ex.what();
        }
    }

    if ( rb->events() & ~adacquire::SignalObserver::wkEvent_UserEventsMask )
        adacquire::task::instance()->handle_so_event( static_cast< adacquire::SignalObserver::wkEvent >( rb->events() ) );

    document::instance()->setData( *data );
    const auto tp = std::chrono::steady_clock::now();
    using namespace std::chrono_literals;
    if ( ( tp - status.tp_data_handled_ ) > 1000ms ) {
        status.tp_data_handled_ = tp;
        sema_.signal();
    }

    // ================== write trace data to datafile ======================
    do {
        auto accessor = std::make_shared< socfpga::dgmod::data_accessor >( data );
        auto tmp = std::make_shared< adacquire::SignalObserver::DataWriter >( accessor );
        io_service_.post( [=](){ adacquire::task::instance()->handle_write( socfpga::dgmod::TraceObserver::__objid__, tmp ); } );
     } while (0 );
    // ============================================================================
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
