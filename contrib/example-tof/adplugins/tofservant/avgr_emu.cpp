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
#include "data_simulator.hpp"
#include <tofinterface/serializer.hpp>
#include <tofinterface/signalC.h>
#include <tofinterface/rawxfer.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/any.hpp>
#include <iostream>

using namespace tofservant;
static boost::posix_time::ptime __uptime__ = boost::posix_time::microsec_clock::local_time();

avgr_emu::avgr_emu() : interval_( 1000 )
	                 , trigger_event_out_( 0 )
                     , npos_( 0 )
                     , navg_( 1 )
                     , work_( io_service_ )
                     , timer_( io_service_ )
                     , data_simulator_( new data_simulator )
{
}

bool
avgr_emu::peripheral_initialize()
{
    timer_.cancel();
    initiate_timer();
    threads_.push_back( std::thread( boost::bind( &boost::asio::io_service::run, &io_service_ ) ) );
    return true;
}

bool
avgr_emu::peripheral_terminate()
{
    io_service_.stop();
    for ( std::thread& thread: threads_ )
        thread.join();
    return true;
}

bool
avgr_emu::peripheral_async_apply_method( const TOF::ControlMethod& m )
{
    set_interval( m.analyzer.sampling_interval );
    set_resolving_power( m.analyzer.resolving_power );
    set_num_average( m.analyzer.number_of_average );
    return true;
}

bool
avgr_emu::peripheral_event_out( unsigned long ev )
{
	// loopback event ( event_out --> out event on TTL hardware ==> will back into event_in )
	trigger_event_out_ |= ev;
    return true;
}


void
avgr_emu::set_interval( std::size_t milliseconds )
{
    interval_ = milliseconds;
}

void
avgr_emu::initiate_timer()
{
    if ( interval_ ) {
        timer_.expires_from_now( boost::posix_time::milliseconds( interval_ ) );
        timer_.async_wait( boost::bind( &avgr_emu::handle_timeout, this, boost::asio::placeholders::error ) );
    }
}

void
avgr_emu::handle_timeout( const boost::system::error_code& error )
{
    if ( !error && interval_ ) {

        initiate_timer();

        ++npos_;
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();

        // data_simulator_->generate_spectrum( navg_ );
        std::shared_ptr< TOFSignal::tofDATA > pDATA( new TOFSignal::tofDATA );
        TOFSignal::tofDATA &d = *pDATA;
        d.sequenceNumber = npos_;
        d.rtcTimeStamp   = time(0);
        d.clockTimeStamp = ( now - __uptime__ ).total_microseconds();

		d.wellKnownEvents = trigger_event_out_;
		trigger_event_out_ = 0;
        
		d.methodId = 0;
        d.numberOfProfiles = 1; // resize d.data_
        d.data.length( 1 );
        TOFSignal::datum& datum = d.data[ 0 ];
        const size_t nbrSamples = data_simulator::ndata;
        datum.values.length( nbrSamples );
		data_simulator_->generate_spectrum( npos_, navg_, datum.values.get_buffer(), nbrSamples );

		toftask::instance()->io_service().post( std::bind(&toftask::handle_profile_data, toftask::instance(), pDATA ) );
    }
}

void
avgr_emu::set_resolving_power( std::size_t rp )
{
    double width = 100.0 / double(rp);  // assume width @ m/z 100
    data_simulator_->peakwidth( width / 2 );
}

void
avgr_emu::set_num_average( std::size_t n )
{
    navg_ = n;
}

