/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "scanlaw.hpp"
#include "constants.hpp"
#include <adportable/float.hpp>
#include <cmath>
#include <cassert>

using namespace accutof::spectrometer;

ScanLaw::~ScanLaw()
{
}

ScanLaw::ScanLaw( double acceleratorVoltage
                  , double tDelay  ) : adportable::TimeSquaredScanLaw( acceleratorVoltage, tDelay )
{
}

ScanLaw::ScanLaw( const ScanLaw& t ) : adportable::TimeSquaredScanLaw( t )
{
}

ScanLaw&
ScanLaw::operator = ( const ScanLaw& t )
{
    static_cast< adportable::TimeSquaredScanLaw& >(*this) = static_cast< const adportable::TimeSquaredScanLaw >( t );
    return *this;
}

// TimeSquaredScanLaw
double
ScanLaw::tDelay() const
{
    return adportable::TimeSquaredScanLaw::tDelay();
}

double
ScanLaw::kAcceleratorVoltage() const
{
    return adportable::TimeSquaredScanLaw::kAcceleratorVoltage();
}

void
ScanLaw::setAcceleratorVoltage( double v )
{
    adportable::TimeSquaredScanLaw::setAcceleratorVoltage( v );
}

void
ScanLaw::setTDelay( double t )
{
    adportable::TimeSquaredScanLaw::setTDelay( t );
}

double
ScanLaw::acceleratorVoltage( double mass, double time, int mode, double tDelay )
{
    return adportable::TimeSquaredScanLaw::acceleratorVoltage( mass, time, mode, tDelay );
}

double
ScanLaw::getMass( double t, int mode ) const
{
    return adportable::TimeSquaredScanLaw::getMass( t, fLength( mode ) );
}

double
ScanLaw::getTime( double m, int mode ) const
{
    return adportable::TimeSquaredScanLaw::getTime( m, fLength( mode ) );
}

double
ScanLaw::getMass( double t, double fLength ) const
{
    return adportable::TimeSquaredScanLaw::getMass( t, fLength );
}

double
ScanLaw::getTime( double m, double fLength ) const
{
    return adportable::TimeSquaredScanLaw::getTime( m, fLength );
}

double
ScanLaw::fLength( int mode ) const
{
    return 2.0;
}
