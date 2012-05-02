// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#include "peakmethod.hpp"
#include <adportable/float.hpp>

using namespace adcontrols;
using namespace adcontrols::chromatography;

PeakMethod::~PeakMethod(void)
{
}

PeakMethod::PeakMethod(void) : minimumHeight_(10)
                             , minimumArea_(0)
                             , minimumWidth_( 0.01 ) // min := 0.6sec
                             , doubleWidthTime_( 0.0 ) // min
                             , slope_( 0.0 ) // uV/min
                             , drift_( 0.0 ) // uV/min
                             , t0_( 0 )
                             , pharmacopoeia_( ePHARMACOPOEIA_NotSpcified )
                             , peakWidthMethod_( ePeakWidth_HalfHeight )
                             , theoreticalPlateMethod_( ePeakWidth_HalfHeight )
{
}

PeakMethod::PeakMethod(const PeakMethod & t ) 
{
    operator=(t);
}

PeakMethod&
PeakMethod::operator = ( const PeakMethod & rhs )
{
    minimumHeight_          = rhs.minimumHeight_;
    minimumArea_            = rhs.minimumArea_;
    minimumWidth_           = rhs.minimumWidth_;
    doubleWidthTime_        = rhs.doubleWidthTime_;
    slope_                  = rhs.slope_;
    drift_                  = rhs.drift_;
    t0_                     = rhs.t0_;
    pharmacopoeia_          = rhs.pharmacopoeia_;
    peakWidthMethod_        = rhs.peakWidthMethod_;
    theoreticalPlateMethod_ = rhs.theoreticalPlateMethod_;
    return * this;
}

bool
PeakMethod::operator == ( const PeakMethod & rhs ) const
{
    using adportable::compare;

    if ( compare<double>::approximatelyEqual( minimumHeight_, rhs.minimumHeight_ ) &&
         compare<double>::approximatelyEqual( minimumArea_, rhs.minimumArea_ ) &&
         compare<double>::approximatelyEqual( minimumWidth_, rhs.minimumWidth_ ) &&
         compare<double>::approximatelyEqual( doubleWidthTime_, rhs.doubleWidthTime_ ) &&
         compare<double>::approximatelyEqual( slope_, rhs.slope_ ) &&
         compare<double>::approximatelyEqual( drift_, rhs.drift_ ) &&
         compare<double>::approximatelyEqual( t0_, rhs.t0_ ) &&
         ( pharmacopoeia_ == rhs.pharmacopoeia_ ) &&
         ( peakWidthMethod_ == rhs.peakWidthMethod_ ) &&
         ( theoreticalPlateMethod_ == rhs.theoreticalPlateMethod_ ) ) {
        return true;
    }
    return false;
}

bool
PeakMethod::operator != ( const PeakMethod & rhs ) const
{
    return !(*this == rhs);
}


double
PeakMethod::minimumHeight() const
{
    return minimumHeight_;
}

void
PeakMethod::minimumHeight( double t )
{
    minimumHeight_ = t;
}

double
PeakMethod::minimumArea() const
{
    return minimumArea_;
}

void
PeakMethod::minimumArea( double t )
{
    minimumArea_ = t;
}

double
PeakMethod::minimumWidth() const
{
    return minimumWidth_;
}

void
PeakMethod::minimumWidth( double t )
{
    minimumWidth_ = t;
}

double
PeakMethod::doubleWidthTime() const
{
    return doubleWidthTime_;
}

void
PeakMethod::doubleWidthTime( double t )
{
    doubleWidthTime_ = t;
}

double
PeakMethod::slope() const
{
    return slope_;
}

void
PeakMethod::slope( double t )
{
    slope_ = t;
}

double
PeakMethod::drift() const
{
    return drift_;
}

void
PeakMethod::drift( double t )
{
    drift_ = t;
}

double
PeakMethod::t0() const
{
    return t0_;
}

void
PeakMethod::t0( double t )
{
    t0_ = t;
}

chromatography::ePharmacopoeia
PeakMethod::pharmacopoeia() const
{
    return pharmacopoeia_;
}

void
PeakMethod::pharmacopoeia( chromatography::ePharmacopoeia t )
{
    pharmacopoeia_ = t;
}

chromatography::ePeakWidthMethod
PeakMethod::peakWidthMethod() const
{
    return peakWidthMethod_;
}

void
PeakMethod::peakWidthMethod( chromatography::ePeakWidthMethod t )
{
    peakWidthMethod_ = t;
}

chromatography::ePeakWidthMethod
PeakMethod::theoreticalPlateMethod() const
{
    return theoreticalPlateMethod_;
}

void
PeakMethod::theoreticalPlateMethod( chromatography::ePeakWidthMethod t )
{
    theoreticalPlateMethod_ = t;
}


