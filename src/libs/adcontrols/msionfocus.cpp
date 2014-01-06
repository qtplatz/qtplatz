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

#include "msionfocus.hpp"
#include "mspeaks.hpp"
#include "mspeak.hpp"

using namespace adcontrols;

MSIonFocus::~MSIonFocus()
{
}

MSIonFocus::MSIonFocus() : acceleratorVoltage_( 0 )
                         , timeOffset_( 0 )
                         , hasCalibration_( false )
                         , mode_( 0 )
                         , fLength_( 0 )
                         , toleranceMethod_( ToleranceInPeakWidth )
{
}

MSIonFocus::MSIonFocus( const MSIonFocus& t )
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
MSIonFocus::acceleratorVoltage() const
{
    return acceleratorVoltage_;
}

double
MSIonFocus::timeOffset() const
{
    return timeOffset_;
}

bool
MSIonFocus::hasCalibration() const
{
    return hasCalibration_;
}

int32_t
MSIonFocus::mode() const
{
    return mode_;
}

double
MSIonFocus::fLength() const
{
    return fLength_;
}

MSIonFocus::eTolerance
MSIonFocus::toleranceMethod() const
{
    return toleranceMethod_;
}

double
MSIonFocus::tolerance( eTolerance m ) const
{
    return tolerances_[ m ];
}

void
MSIonFocus::acceleratorVoltage( double v )
{
    acceleratorVoltage_ = v;
}

void
MSIonFocus::timeOffset( double v )
{
    timeOffset_ = v;
}

void
MSIonFocus::hasCalibration( bool v )
{
    hasCalibration_ = v;
}

void
MSIonFocus::mode( int32_t v )
{
    mode_ = v;
}

void
MSIonFocus::fLength( double v )
{
    fLength_ = v;
}

void
MSIonFocus::toleranceMethod( eTolerance v )
{
    toleranceMethod_ = v;
}

void
MSIonFocus::tolerance( eTolerance t, double v )
{
    tolerances_[ t ] = v;
}

const MSPeaks& 
MSIonFocus::expected() const
{
    return expected_;
}

const MSPeaks&
MSIonFocus::assigned() const
{
    return assigned_;
}

MSPeaks&
MSIonFocus::expected()
{
    return expected_;
}

MSPeaks&
MSIonFocus::assigned()
{
    return assigned_;
}

