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

#include "spectrometer.h"
#include "interpreter.h"
#include <adcontrols/visitor.h>
#include <cmath>

using namespace tofspectrometer;

Spectrometer * Spectrometer::instance_ = 0;

adcontrols::MassSpectrometer *
Spectrometer::instance()
{
    if ( instance_ == 0 ) {
        instance_ = new tofspectrometer::Spectrometer();
        atexit( tofspectrometer::Spectrometer::dispose );
    }
    return instance_;
}

void
Spectrometer::dispose()
{
   if ( instance_ )
       delete instance_;
   instance_ = 0;
}

Spectrometer::~Spectrometer()
{
    delete pScanLaw_;
    delete pInterpreter_;
}

Spectrometer::Spectrometer() : pScanLaw_(0)
                             , pInterpreter_(0)
{
    pScanLaw_ = new MultiTurnScanLaw( 0.01389, /* delay */ 0.0, /* FT(V) */ 5000 );
    pInterpreter_ = new Interpreter();
}

void
Spectrometer::accept( adcontrols::Visitor& visitor )
{
    visitor.visit(*this);
}

adcontrols::MassSpectrometer::factory_type
Spectrometer::factory()
{
    return &Spectrometer::instance;
}

const wchar_t *
Spectrometer::name() const
{
    return L"tofSpectrometer";
}

const adcontrols::MassSpectrometer::ScanLaw&
Spectrometer::getScanLaw() const
{
    return *pScanLaw_;
}

const adcontrols::DataInterpreter&
Spectrometer::getDataInterpreter() const
{
    return *pInterpreter_;
}


/////////////////////////////////////////////

MultiTurnScanLaw::MultiTurnScanLaw( double timeCoefficient, double timeDelay, double acclVolt )
: timeCoefficient_( timeCoefficient )
, timeDelay_( timeDelay )
, acclVoltage_( acclVolt )
{
}

double
MultiTurnScanLaw::getMass( double tof, int nTurn ) const
{
    return getMass( tof, 0.43764 + nTurn * 0.66273 );
}

double
MultiTurnScanLaw::getTime( double mass, int nTurn ) const
{
    return getTime( mass, 0.43764 + nTurn * 0.66273 );
}

double
MultiTurnScanLaw::getMass( double tof, double fLength ) const
{
    double t = tof / fLength - timeDelay_;
    double m = ( ( timeCoefficient_ * timeCoefficient_ ) * ( t * t ) ) * acclVoltage_;
    return m;
}

double
MultiTurnScanLaw::getTime( double mass, double fLength ) const
{
    double v = std::sqrt( acclVoltage_ / mass ) * timeCoefficient_; // (m/s)
    return fLength * ( 1.0 / v ) + timeDelay_;
}

////////////////////////////////////////////////////////////////////