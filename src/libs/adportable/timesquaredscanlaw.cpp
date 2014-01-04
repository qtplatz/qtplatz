/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "timesquaredscanlaw.hpp"
#include <cmath>

using namespace adportable;

TimeSquaredScanLaw::TimeSquaredScanLaw( double kAcceleratorVoltage
                                        , double tDelay
                                        , double fLength )
    : kAcceleratorVoltage_( kAcceleratorVoltage )
    , tDelay_( tDelay )
	, kTimeSquaredCoeffs_( 2.0 * kELEMENTAL_CHARGE / kATOMIC_MASS_CONSTANT )
    , fLength_( fLength )
{
}

TimeSquaredScanLaw::TimeSquaredScanLaw( const TimeSquaredScanLaw& t )
    : kAcceleratorVoltage_( t.kAcceleratorVoltage_ )
    , tDelay_( t.tDelay_ )
    , kTimeSquaredCoeffs_( t.kTimeSquaredCoeffs_ )
    , fLength_( t.fLength_ )
{
}

double
TimeSquaredScanLaw::getMass( double time, int mode ) const
{
    return getMass( time, fLength( mode ) );
}

double
TimeSquaredScanLaw::getTime( double mass, int mode ) const
{
    return getTime( mass, fLength( mode ) );
}

double
TimeSquaredScanLaw::getMass( double time, double fLength ) const
{
    double k = ( kTimeSquaredCoeffs * kAcceleratorVoltage_ ) / ( fLength * fLength );
    double t = time - tDelay_;
    double mass = k * ( t * t );
	return mass;
}

double
TimeSquaredScanLaw::getTime( double mass, double fLength ) const
{
    double k = ( kTimeSquaredCoeffs * kAcceleratorVoltage_ ) / ( fLength * fLength );
	double time = std::sqrt(mass / k) + tDelay_;
	return time;
}

double
TimeSquaredScanLaw::fLength( int mode ) const
{
    // This method should be orverridden for MULTITURN spectrometer or more than 2 mode
    // analyzer spectrometer (ex. Linear|Refrectron mode for MALDI TOF spectrometer)
	(void)mode;
    return fLength_; 
}

double
TimeSquaredScanLaw::acceleratorVoltage( double mass, double time, int mode, double tDelay )
{
    const double L = fLength( mode );
    return acceleratorVoltage( mass, time, L, tDelay );
}

double
TimeSquaredScanLaw::acceleratorVoltage( double mass, double time, double fLength, double tDelay )
{
    const double t = time - tDelay;
    double acceleratorVoltage = ( mass * ( fLength * fLength ) ) / ( kTimeSquaredCoeffs * ( t * t ) );
    return acceleratorVoltage;
}
