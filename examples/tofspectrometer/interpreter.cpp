// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "interpreter.h"

# pragma warning(disable: 4996)
#  include <adinterface/signalobserverC.h>
#  include "../tofcontroller/tofcontrollerC.h"
# pragma warning(default: 4996)

#include <adcontrols/massspectrometer.h>
#include <adcontrols/massspectrum.h>
#include <adcontrols/traceaccessor.h>
#include <adcontrols/descriptions.h>
#include <sstream>
#include <boost/format.hpp>

using namespace tofspectrometer;

Interpreter::~Interpreter()
{
}

Interpreter::Interpreter()
{
}

bool
Interpreter::translate( adcontrols::MassSpectrum& ms
                       , const SignalObserver::DataReadBuffer& rb
                       , const adcontrols::MassSpectrometer& spectrometer
                       , size_t idData ) const
{
    const adcontrols::MassSpectrometer::ScanLaw& scanLaw = spectrometer.getScanLaw();

    std::wostringstream o;
    o << boost::wformat(L"Spectrum pos[%1%] EV:%2%") % rb.pos % rb.events;

    size_t delay = 0;
    size_t sampInterval = 500;
    size_t nbrSamples = 0;
    unsigned long wellKnownEvents = 0;
    size_t tmstamp = 0;
    size_t numSeg = 1;

    do {
        SignalObserver::AveragerData *pavgr;
        if ( rb.method >>= pavgr ) {
            delay = pavgr->startDelay;
            sampInterval = pavgr->sampInterval;
            o << boost::wformat(L" delay:%1% nbrSamples: %2%") % pavgr->startDelay % pavgr->nbrSamples;
        }
    } while(0);

    TOFInstrument::AveragerData * plocal;
    if ( rb.method >>= plocal ) {
        delay = plocal->startDelay;
        sampInterval = plocal->sampInterval;
        nbrSamples = plocal->nbrSamples;
        wellKnownEvents = plocal->wellKnownEvents;
        tmstamp = plocal->usec;
        // numSeg = plocal->segments.length();

        o << boost::wformat(L" delay:%1% nbrSamples: %2% #seg: %3%") % delay % nbrSamples % numSeg;
    }

    ms.addDescription( adcontrols::Description( L"acquire.title", o.str() ) );

    const size_t nsize = rb.array.length();
    ms.resize( nsize );
    boost::scoped_array<double> pX( new double [ nsize ] );
    boost::scoped_array<double> pY( new double [ nsize ] );
    for ( size_t i = 0; i < nsize; ++i ) {
        double tof = ( delay + double( sampInterval * i ) ) / 1000000; // ps -> us
        pX[i] = scanLaw.getMass( tof, 0.688 );
        pY[i] = rb.array[i];
    }
    ms.setMassArray( pX.get(), true ); // update acq range
    ms.setIntensityArray( pY.get() );

    if ( idData == 0 )
        return true;
    return false;
}

bool
Interpreter::translate( adcontrols::TraceAccessor& accessor
                       , const SignalObserver::DataReadBuffer& rb ) const
{
    return false;
}