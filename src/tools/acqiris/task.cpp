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

#include "acqiris_method.hpp"
#include "acqiris_protocol.hpp"
#include "digitizer.hpp"
#include "document.hpp"
#include "task.hpp"
#include "tcp_server.hpp"
#include "waveform.hpp"
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
             , tp_data_handled_( std::chrono::system_clock::now() )
             , tp_uptime_( std::chrono::system_clock::now() )
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

            document::instance()->connect_prepare( boost::bind( &task::prepare_for_run, this, _1, _2 ) );
            document::instance()->connect_finalize( boost::bind( &task::finalize, this ) );
            
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
task::prepare_for_run( std::shared_ptr< const aqdrv4::acqiris_method > m, aqdrv4::SubMethodType )
{
    strand_.post( [=] {
            auto adapted = task::digitizer()->digitizer_setup( m );
            document::instance()->acqiris_method_adapted( adapted );
        } );

    if ( !std::atomic_flag_test_and_set( &acquire_posted_) )
        strand_.post( [=] { acquire(); } );

    if ( auto server = document::instance()->server() ) {
        if ( auto data = aqdrv4::protocol_serializer::serialize( *m ) )
            server->post( data );
    }
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
        
        if ( digitizer()->waitForEndOfAcquisition( 3000 ) == digitizer::success ) {
            
            static const int nbrADCBits = digitizer()->nbrADCBits();
            auto d = std::make_shared< aqdrv4::waveform >( ( nbrADCBits > 8 ) ? sizeof( int16_t ) : sizeof( int8_t ) );

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
            
            d->delayTime() = digitizer()->delayTime();
            d->serialNumber() = serialCounter_++;

            document::instance()->push( std::move( d ) );

            auto tp = std::chrono::system_clock::now();
            if ( tp - tp_data_handled_ > 200ms ) {
                emit document::instance()->updateData();
                tp_data_handled_ = tp;
            }
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

        strand_.post( [] {
                int temp = digitizer()->readTemperature();
                document::instance()->replyTemperature( temp );
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
            task::instance()->prepare_for_run( document::instance()->acqiris_method(), aqdrv4::allMethod );
        }
    }
    return true;
}
