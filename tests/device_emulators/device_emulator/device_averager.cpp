// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
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

#include "device_averager.h"
#include <ace/Message_Block.h>
#include <acewrapper/inputcdr.h>
#include <acewrapper/outputcdr.h>
#pragma warning(disable:4996)
#include "../tofcontroller/tofcontrollerC.h"
#pragma warning(default:4996)
#include "../tofcontroller/constants.h"
#include "devicefacade.h"
#include "constants.h"
#include "./reactor_thread.h"
#include <ace/Reactor.h>
#include <adportable/protocollifecycle.h>
#include "txtspectrum.h"

using namespace device_emulator;

device_averager::~device_averager(void)
{
    if ( state_ > device_state::state_initializing )
        deactivate();
}

device_averager::device_averager(void) : uptime_(0)
{
}

device_averager::device_averager( const device_averager& t ) : device_state( t ), uptime_(0)
{
}

bool
device_averager::instruct_handle_data( ACE_InputCDR& cdr, unsigned long cmdId )
{
    ACE_UNUSED_ARG( cdr );
	if ( cmdId == tofcontroller::constants::SESSION_SENDTO_DEVICE ) {
		// unsigned long clsId;
		//cdr.read_ulong( clsId );
	}
	return false;
}

bool
device_averager::instruct_copy_data( ACE_OutputCDR&, ACE_InputCDR&, unsigned long )
{
    return false;
}

void
device_averager::activate()
{
    doit( device_state::command_initialize );
    ACE_Reactor * reactor = acewrapper::singleton::ReactorThread::instance()->get_reactor();
    reactor->schedule_timer( this, 0, ACE_Time_Value(1), ACE_Time_Value(1) );
}

void
device_averager::deactivate()
{
    doit( device_state::command_off );
    ACE_Reactor * reactor = acewrapper::singleton::ReactorThread::instance()->get_reactor();
    reactor->cancel_timer( this );
}

int
device_averager::handle_timeout( const ACE_Time_Value& tv, const void * )
{
    if ( state() <= device_state::state_initializing )
        doit( device_state::command_stop );

    if ( uptime_.sec() == 0 && uptime_.usec() == 0 )
        uptime_ = tv;
    
    size_t hLen = 32;
    size_t nbrSamples = 1024 * 15;
    size_t nbrAverage = 1;
    size_t sampInterval = 500;
    size_t nDelay = 12 * 1000000 / 500;
    unsigned long wellKnownEvents = 0;

	static size_t npos;

	const std::vector< TXTSpectrum > spectra = device_emulator::singleton::device_facade::instance()->test_spectra();
    const TXTSpectrum * psp = 0;
	if ( ! spectra.empty() ) {
		const TXTSpectrum& sp = spectra[0];
		nbrSamples = sp.iarray_.size();
        sampInterval = sp.sampInterval_;
        nDelay = sp.startDelay_;
		psp = &sp;
	}
	size_t wformLen = (nbrSamples * 3 / 4) + 1; // 32bit -> 24bit 

	ACE_Message_Block * mb = new ACE_Message_Block( adportable::protocol::LifeCycle::wr_offset() + ((hLen + wformLen) * sizeof(long)));
	size_t size = mb->size();
	memset( mb->wr_ptr(), 0, size );
	mb->wr_ptr( adportable::protocol::LifeCycle::wr_offset() );

	long * pmeta = reinterpret_cast<long *>(mb->wr_ptr());
	// long * pdata = pmeta + hLen;
    unsigned char * pchar = reinterpret_cast<unsigned char *>( pmeta + hLen );
	mb->wr_ptr( mb->size() );

    ACE_Time_Value tm = tv - uptime_;
    unsigned long long uptime = tm.sec() * 1000000 + tm.usec();

	*pmeta++ = TOFConstants::ClassID_ProfileData;
	*pmeta++ = npos++;
	*pmeta++ = unsigned long ( uptime & 0xffffffff ); // time since inject, to do
	*pmeta++ = unsigned long ( uptime & 0xffffffff );
	*pmeta++ = unsigned long ( uptime >> 32 );
	*pmeta++ = nbrSamples;
    *pmeta++ = nbrAverage;
	*pmeta++ = nDelay;
	*pmeta++ = sampInterval;
    *pmeta++ = wellKnownEvents;

	// simulate noise
	srand( int(tv.sec()) );

	if ( psp ) {
		double f = 1000.0 / psp->maxValue_;
		for ( size_t i = 0; i < nbrSamples; ++i ) {
            double d = psp->iarray_[i] + (psp->maxValue_ / 20.0);
			if ( d < (-psp->maxValue_ / 20) )
				d = (-psp->maxValue_ / 20);
			long x = ( d * f ) + ( double(rand()) * 10 / RAND_MAX );
			*pchar++ = x >> 16;
			*pchar++ = x >> 8;
            *pchar++ = x;
		}
	}
	// todo: overlay chemical background, and sample peak
	mb->msg_type( constants::MB_DATA_TO_CONTROLLER );
	singleton::device_facade::instance()->putq( mb );

	return 0;
}