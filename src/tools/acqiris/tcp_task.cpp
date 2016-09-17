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

#include "tcp_task.hpp"
#include "tcp_client.hpp"
#include "document.hpp"
#include "waveform.hpp"
#include <iostream>

using namespace aqdrv4::client;

tcp_task::~tcp_task()
{
}

tcp_task::tcp_task() : worker_stopping_( false )
                     , work_( io_service_ )
                     , strand_( io_service_ )
                     , timer_( io_service_ )
                     , tp_data_handled_( std::chrono::system_clock::now() )
{
    acquire_posted_.clear();
}

tcp_task *
tcp_task::instance()
{
    static tcp_task __instance;
    return &__instance;
}

bool
tcp_task::initialize()
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
tcp_task::finalize()
{
    worker_stopping_ = true;
    
    sema_.signal();
    
    io_service_.stop();
    
    for ( auto& t: threads_ )
        t.join();

    return true;
}

void
tcp_task::worker_thread()
{
    do {
        sema_.wait();
        if ( worker_stopping_ )
            return;
        
    } while ( true );
}

void
tcp_task::prepare_for_run( std::shared_ptr< const aqdrv4::acqiris_method > m )
{
    // strand_.post( [=] {
    //         auto adapted = digitizer->digitizer_setup( m );
    //         document::instance()->acqiris_method_adapted( adapted );
    //     } );
    
    // if ( !std::atomic_flag_test_and_set( &acquire_posted_) )
    //     strand_.post( [=] { acquire( digitizer ); } );
}

void
tcp_task::push( response_type v )
{
}

#if 0
void
tcp_task::acquire( digitizer * digitizer )
{
    static int count = 0;

    if ( std::atomic_flag_test_and_set( &acquire_posted_ ) )
        strand_.post( [=] { acquire( digitizer ); } );    // scedule for next acquire
    else
        std::atomic_flag_clear( &acquire_posted_ ); // keep it false
    
    using namespace std::chrono_literals;
}
#endif

void
tcp_task::handle_timer( const boost::system::error_code& ec )
{
    using namespace std::chrono_literals;
    
    if ( ec != boost::asio::error::operation_aborted ) {
#if 0        
        strand_.post( [] {
                std::cout << "readData" << std::endl;
                if ( auto client = document::instance()->client() )
                    client->readData();
            } );
#endif
        boost::system::error_code erc;
        timer_.expires_from_now( 3s, erc );
        if ( !erc )
            timer_.async_wait( [&]( const boost::system::error_code& ec ){ handle_timer( ec ); });
    }
}
