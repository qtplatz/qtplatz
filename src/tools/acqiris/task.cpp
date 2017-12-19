/**************************************************************************
** Copyright (C) 2014-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "digitizer.hpp"
#if ! ACQIRIS_DAEMON
# include "document.hpp"
#endif
#include "task.hpp"
#include "tcp_server.hpp"
#include <acqrscontrols/acqiris_waveform.hpp>
#include <acqrscontrols/acqiris_method.hpp>
#include <acqrscontrols/acqiris_protocol.hpp>
#include <adacquire/constants.hpp>
#include <adportable/debug.hpp>
#include <boost/bind.hpp>
#include <iostream>

task::~task()
{
}

task::task() : worker_stopping_( false )
             , work_( io_service_ )
             , strand_( io_service_ )
             , timer_( io_service_ )
             , tp_uptime_( std::chrono::system_clock::now() )
             , tp_data_handled_( tp_uptime_ )
             , inject_timepoint_( 0 )
             , inject_serialnumber_( 0 )
             , inject_requested_( false )
             , acquisition_active_( false )
    
{
    acquire_posted_.clear();
}

task *
task::instance()
{
    static task __instance;
    return &__instance;
}

class digitizer *
task::digitizer()
{
    static class digitizer __acqiris;
    return &__acqiris;
}

bool
task::initialize()
{
    using namespace std::chrono_literals;

    static std::once_flag flag;

    std::call_once( flag, [&]() {

            acquire_posted_.clear();

            threads_.emplace_back( std::thread( [&]{ worker_thread(); } ) );

            unsigned nCores = std::thread::hardware_concurrency();
            while( nCores-- )
                threads_.emplace_back( std::thread( [&]{ io_service_.run(); } ) );

            timer_.expires_from_now( 5s );
            timer_.async_wait( [&]( const boost::system::error_code& ec ){ handle_timer(ec); } );

        } );

    return true;
}

bool
task::finalize()
{
    worker_stopping_ = true;

    sema_.signal();

    io_service_.stop();

    for ( auto& t: threads_ )
        t.join();

    return true;
}

void
task::worker_thread()
{
    do {
        sema_.wait();
        if ( worker_stopping_ )
            return;
        
    } while ( true );
}

void
task::prepare_for_run( std::shared_ptr< const acqrscontrols::aqdrv4::acqiris_method > m, acqrscontrols::aqdrv4::SubMethodType )
{
    strand_.post( [=] {
            auto adapted = task::digitizer()->digitizer_setup( m );

            emit_acqiris_method_adapted_( adapted );

            methodNumber_ = m->methodNumber_;
        } );

    if ( !std::atomic_flag_test_and_set( &acquire_posted_) )
        strand_.post( [=] { acquire(); } );
}

void
task::event_out( uint32_t events )
{
    if ( events == adacquire::Instrument::instEventInjectOut )
        strand_.post( [=] { inject_requested_ = true; });
}

void
task::acquire()
{
    static int count = 0;

    if ( std::atomic_flag_test_and_set( &acquire_posted_ ) )
        strand_.post( [=] { acquire(); } );    // scedule for next acquire
    else
        std::atomic_flag_clear( &acquire_posted_ ); // keep it false
    
    using namespace std::chrono_literals;

    if ( task::digitizer()->acquire() ) {

        auto tp_trig = std::chrono::system_clock::now();
        
        if ( digitizer()->waitForEndOfAcquisition( 3000 ) == digitizer::success ) {
            
            static const int nbrADCBits = digitizer()->nbrADCBits();
            auto d = std::make_shared< acqrscontrols::aqdrv4::waveform >( ( nbrADCBits > 8 ) ? sizeof( int16_t ) : sizeof( int8_t ) );
            d->setMethodNumber( methodNumber_ );

            bool success;
            if ( nbrADCBits <= 8 ) {
                success = digitizer()->readData<int8_t>( 1, d->dataDesc(), d->segDesc(), d->d() );
            } else {
                success = digitizer()->readData<int16_t>( 1, d->dataDesc(), d->segDesc(), d->d() );
            }
            
            if ( success && digitizer()->isSimulated() ) {
                uint64_t ps = std::chrono::nanoseconds( std::chrono::system_clock::now() - tp_uptime_ ).count() * 1000;
                d->segDesc().timeStampLo = int32_t( ps );
                d->segDesc().timeStampHi = int32_t( ps >> 32 );
            }

            static uint64_t serialCounter_ = 0;

            d->serialNumber() = serialCounter_++;
            if ( inject_requested_ ) {
                d->wellKnownEvents() |= adacquire::SignalObserver::wkEvent_INJECT;
                inject_timepoint_    = d->timeStamp(); // ps
                inject_serialnumber_ = d->serialNumber();
                inject_requested_    = false;
            }
            d->delayTime()          = digitizer()->delayTime();
            d->timeSinceInject()    = ( d->timeStamp() - inject_timepoint_ ) / 1000; // ps -> ns
            d->serialNumber0()      = inject_serialnumber_;
            d->timeSinceEpoch()     = std::chrono::nanoseconds( tp_trig.time_since_epoch() ).count();

            push_handler_( std::move( d ) );

        } else {
            ADDEBUG() << "acquire timed out " << count++;
        }
    } else {
        digitizer()->stop();
        ADDEBUG() << "acquire failed. " << count++;
        std::this_thread::sleep_for( 1s );
    }
}

void
task::handle_timer( const boost::system::error_code& ec )
{
    using namespace std::chrono_literals;
    
    if ( ec != boost::asio::error::operation_aborted ) {

        strand_.post( [this] {
                int temp = digitizer()->readTemperature();
                emit_replyTemperature_( temp );
            } );

        boost::system::error_code erc;
        timer_.expires_from_now( 5s, erc );
        if ( !erc )
            timer_.async_wait( [&]( const boost::system::error_code& ec ){ handle_timer( ec ); });
    }
}

bool
task::digitizer_initialize()
{
    task::instance()->initialize();
    
    auto aqrs = digitizer();
    
    if ( aqrs->initialize() ) {
        if ( aqrs->findDevice() ) {
            return true;
        }
    }
    return true;
}

boost::signals2::connection
task::connect_acqiris_method_adapted( const acqiris_method_adapted_t::slot_type & subscriber )
{
    return emit_acqiris_method_adapted_.connect( subscriber );
}

boost::signals2::connection
task::connect_replyTemperature( const replyTemperature_t::slot_type & subscriber )
{
    return emit_replyTemperature_.connect( subscriber );
}

void
task::connect_push( std::function< void( std::shared_ptr< acqrscontrols::aqdrv4::waveform > ) > f )
{
    push_handler_ = f;
}

