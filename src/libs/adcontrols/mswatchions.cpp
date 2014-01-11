/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mswatchions.hpp"
#include "mspeaks.hpp"
#include "mspeak.hpp"

using namespace adcontrols;

MSWatchIons::~MSWatchIons()
{
}

MSWatchIons::MSWatchIons() : acceleratorVoltage_( 0 )
                         , timeOffset_( 0 )
                         , hasCalibration_( false )
                         , mode_( 0 )
                         , fLength_( 0 )
                         , toleranceMethod_( ToleranceInPeakWidth )
{
}

MSWatchIons::MSWatchIons( const MSWatchIons& t )
    : acceleratorVoltage_( t.acceleratorVoltage_ )
    , timeOffset_( t.timeOffset_ )
    , hasCalibration_( t.hasCalibration_ )
    , mode_( t.mode_ )
    , fLength_( t.fLength_ )
    , toleranceMethod_( t.toleranceMethod_ )
    , tolerances_( t.tolerances_ )
    , expected_( t.expected_ )
    , assigned_( t.assigned_ )
{
}

double
MSWatchIons::acceleratorVoltage() const
{
    return acceleratorVoltage_;
}

double
MSWatchIons::timeOffset() const
{
    return timeOffset_;
}

bool
MSWatchIons::hasCalibration() const
{
    return hasCalibration_;
}

int32_t
MSWatchIons::mode() const
{
    return mode_;
}

double
MSWatchIons::fLength() const
{
    return fLength_;
}

MSWatchIons::eTolerance
MSWatchIons::toleranceMethod() const
{
    return toleranceMethod_;
}

double
MSWatchIons::tolerance( eTolerance m ) const
{
    return tolerances_[ m ];
}

void
MSWatchIons::acceleratorVoltage( double v )
{
    acceleratorVoltage_ = v;
}

void
MSWatchIons::timeOffset( double v )
{
    timeOffset_ = v;
}

void
MSWatchIons::hasCalibration( bool v )
{
    hasCalibration_ = v;
}

void
MSWatchIons::mode( int32_t v )
{
    mode_ = v;
}

void
MSWatchIons::fLength( double v )
{
    fLength_ = v;
}

void
MSWatchIons::toleranceMethod( eTolerance v )
{
    toleranceMethod_ = v;
}

void
MSWatchIons::tolerance( eTolerance t, double v )
{
    tolerances_[ t ] = v;
}

const MSPeaks& 
MSWatchIons::expected() const
{
    return expected_;
}

const MSPeaks&
MSWatchIons::assigned() const
{
    return assigned_;
}

MSPeaks&
MSWatchIons::expected()
{
    return expected_;
}

MSPeaks&
MSWatchIons::assigned()
{
    return assigned_;
}

