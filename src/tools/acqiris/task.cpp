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

#include "task.hpp"
#include "acqiris.hpp"
#include "document.hpp"
#include "waveform.hpp"
#include <iostream>

task::~task()
{
}

task::task() : worker_stopping_( false )
             , work_( io_service_ )
             , strand_( io_service_ )
             , tp_data_handled_( std::chrono::system_clock::now() )
{
    acquire_posted_.clear();
}

task *
task::instance()
{
    static task __instance;
    return &__instance;
}

bool
task::initialize()
{
    static std::once_flag flag;

    std::call_once( flag, [&]() {

            acquire_posted_.clear();

            threads_.emplace_back( std::thread( [&]{ worker_thread(); } ) );

            unsigned nCores = std::thread::hardware_concurrency();
            while( nCores-- )
                threads_.emplace_back( std::thread( [&]{ io_service_.run(); } ) );
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
task::acquire( acqiris * digitizer )
{
    static int count = 0;

    using namespace std::chrono_literals;

    if ( digitizer->acquire() ) {
        if ( digitizer->waitForEndOfAcquisition( 3000 ) == acqiris::success ) {
            auto d = std::make_shared< waveform >();
            digitizer->readData( 1, d->dataDesc_, d->segDesc_, d->data_ );
            document::instance()->push( std::move( d ) );

            auto tp = std::chrono::system_clock::now();
            if ( tp - tp_data_handled_ > 500ms ) {
                emit document::instance()->updateData();
                tp_data_handled_ = tp;
            }
            // std::cout << "acquire " << count++ << std::endl;
        } else {
            std::cout << "acquire timed out" << std::endl;
        }
    } else {
        std::cout << "acquire failed." << std::endl;
    }
    strand_.post( [&] { acquire( digitizer ); } );    // scedule for next acquire
}
