/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "avgr_emu.hpp"
#include "toftask.hpp"
#include "data_simulator.hpp"
#include "constants.h"
#include <tofinterface/serializer.hpp>
#include <tofinterface/signalC.h>
#include <tofinterface/rawxfer.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/any.hpp>
#include <boost/foreach.hpp>
#include <iostream>

namespace tofservant { namespace avgr {
        class worker {
        public:
            static worker * instance();

            worker( std::size_t interval = 1000 );
            void set_interval( std::size_t milliseconds );
            void set_resolvingpower( std::size_t rp );
            void set_numAverage( std::size_t );
            void start();
            void stop();
            inline boost::asio::io_service& io_service() { return io_service_; }
        private:
            void handle_timeout( const boost::system::error_code& );
            void initiate_timer();
            boost::asio::io_service io_service_;
            std::size_t tid_;
            std::size_t interval_;
            boost::asio::deadline_timer timer_;
            boost::scoped_ptr< data_simulator > data_simulator_;
            size_t npos_;
            size_t navg_;
            static worker * instance_;
        };
    }
}

tofservant::avgr::worker * tofservant::avgr::worker::instance_ = 0;

using namespace tofservant;

avgr_emu::avgr_emu()
{
}

bool
avgr_emu::peripheral_initialize()
{
    avgr::worker * worker = avgr::worker::instance();
    worker->start();

    std::shared_ptr< boost::thread >
        thread( new boost::thread( boost::bind( &boost::asio::io_service::run, &worker->io_service() ) ) );
    threads_.push_back( thread );
                                   
    return true;
}

bool
avgr_emu::peripheral_terminate()
{
    avgr::worker::instance()->stop();

    for ( std::shared_ptr< boost::thread >& thread: threads_ )
        thread->join();

    return true;
}

bool
avgr_emu::peripheral_sync_command( unsigned int, ACE_Message_Block * )
{
    return true;
}

bool
avgr_emu::peripheral_async_command( unsigned int, ACE_Message_Block * )
{
    return true;
}

bool
avgr_emu::peripheral_async_command( unsigned int, const boost::any& any )
{
    using tofinterface::dma_data_t;

    avgr::worker * worker = avgr::worker::instance();

    if ( any.type() == typeid( TOF::ControlMethod ) ) {
        try { 
            const TOF::ControlMethod& m = boost::any_cast< TOF::ControlMethod >( any );
            worker->set_interval( m.analyzer.sampling_interval );
            worker->set_resolvingpower( m.analyzer.resolving_power );
            worker->set_numAverage( m.analyzer.number_of_average );
        } catch ( std::exception& ex ) {
            std::cout << "got an exception: " << ex.what() << std::endl;
        }
    } else {
        std::cout << "fpga::peripheral_async_command: got an unknwon class" << std::endl;
    }
    return true;
}

bool
avgr_emu::putq( ACE_Message_Block * )
{
    return true;
}

///////////
using namespace tofservant::avgr;

static boost::posix_time::ptime __uptime__ = boost::posix_time::microsec_clock::local_time();

worker::worker( std::size_t interval ) : interval_( interval )
                                       , timer_( io_service_ )
                                       , data_simulator_( new data_simulator )
                                       , npos_( 0 )
                                       , navg_( 1 )
{
}

void
worker::start()
{
    timer_.cancel();
    initiate_timer();
}

void
worker::stop()
{
    timer_.cancel();
}

void
worker::set_interval( std::size_t milliseconds )
{
    interval_ = milliseconds;
}

void
worker::initiate_timer()
{
    if ( interval_ ) {
        timer_.expires_from_now( boost::posix_time::milliseconds( interval_ ) );
        timer_.async_wait( boost::bind( &worker::handle_timeout, this, boost::asio::placeholders::error ) );
    }
}

void
worker::handle_timeout( const boost::system::error_code& error )
{
    if ( !error && interval_ ) {

        initiate_timer();

        ++npos_;
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();

        std::cout << "worker::handle_timeout npos: " << npos_ << std::endl;

        data_simulator_->generate_spectrum( navg_ );
 
        TOFSignal::tofDATA d;
        d.sequenceNumber = npos_;
        d.rtcTimeStamp   = time(0);
        d.clockTimeStamp = ( now - __uptime__ ).total_microseconds();
        d.wellKnownEvents = 0;
        d.methodId = 0;
        d.numberOfProfiles = 1; // resize d.data_
        d.data.length( 1 );
        TOFSignal::datum& datum = d.data[ 0 ];
        const size_t nbrSamples = data_simulator_->intensities().size();
        datum.values.length( nbrSamples );
        std::copy( data_simulator_->intensities().begin()
                   , data_simulator_->intensities().end(), &datum.values[0] );

        TAO_OutputCDR cdr;
        cdr << d;
        ACE_Message_Block * mblk = new ACE_Message_Block;
        mblk->cont( cdr.begin()->duplicate() );
        mblk->msg_type( constants::MB_TOF_DATA );
        toftask::instance()->putq( mblk );
    }
}

void
worker::set_resolvingpower( std::size_t rp )
{
    double width = 100.0 / double(rp);  // assume width @ m/z 100
    data_simulator_->peakwidth( width / 2 );
}

void
worker::set_numAverage( std::size_t n )
{
    navg_ = n;
}

worker *
worker::instance()
{
    if ( instance_ == 0 ) {
        instance_ = new worker();
    }
    return instance_;
}
