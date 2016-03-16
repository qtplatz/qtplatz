// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include "mslockmethod.hpp"
#include <adportable/is_equal.hpp>

using namespace adcontrols;

MSLockMethod::~MSLockMethod(void)
{
}

MSLockMethod::MSLockMethod() : enabled_( false )
                             , enablePeakThreshold_( false )
                             , toleranceMethod_( idToleranceDaltons )
                             , algorithm_( idFindLargest )
                             , toleranceDa_( 0.2 )
                             , tolerancePpm_( 10.0 )
                             , peakIntensityThreshold_( 10000.0 )
{
}

MSLockMethod::MSLockMethod(const MSLockMethod & t) : enabled_( t.enabled_ )
                                                   , enablePeakThreshold_( t.enablePeakThreshold_ )
                                                   , toleranceMethod_( t.toleranceMethod_)
                                                   , algorithm_( t.algorithm_ )
                                                   , toleranceDa_( t.toleranceDa_ )
                                                   , tolerancePpm_( t.tolerancePpm_ )
                                                   , peakIntensityThreshold_( t.peakIntensityThreshold_ )
{
}

MSLockMethod&
MSLockMethod::operator=(const MSLockMethod& t)
{
    enabled_ = t.enabled_;
    enablePeakThreshold_ = t.enablePeakThreshold_;
    toleranceMethod_ = t.toleranceMethod_;
    algorithm_ = t.algorithm_;
    toleranceDa_ = t.toleranceDa_;
    tolerancePpm_ = t.tolerancePpm_;
    peakIntensityThreshold_ = t.peakIntensityThreshold_;
    return *this;
}

bool
MSLockMethod::enabled() const
{
    return enabled_;
}

void
MSLockMethod::setEnabled( bool t )
{
    enabled_ = t;
}

bool
MSLockMethod::enablePeakThreshold() const
{
    return enablePeakThreshold_;
}

void
MSLockMethod::setEnablePeakThreshold( bool t )
{
    enablePeakThreshold_ = t;
}

idFindAlgorithm
MSLockMethod::algorithm() const
{
    return algorithm_;
}

void
MSLockMethod::setAlgorithm( idFindAlgorithm t )
{
    algorithm_ = t;
}

idToleranceMethod
MSLockMethod::toleranceMethod() const
{
    return toleranceMethod_;
}

void
MSLockMethod::setToleranceMethod( idToleranceMethod t )
{
    toleranceMethod_ = t;
}

double
MSLockMethod::tolerance( idToleranceMethod t ) const
{
    return t == idToleranceDaltons ? toleranceDa_ : tolerancePpm_;
}

void
MSLockMethod::setTolerance( idToleranceMethod t, double value )
{
    if ( t == idToleranceDaltons )
        toleranceDa_ = value;
    else 
        tolerancePpm_ = value;
}

double
MSLockMethod::peakIntensityThreshold() const
{
    return peakIntensityThreshold_;
}

void
MSLockMethod::setPeakIntensityThreshold( double value )
{
    peakIntensityThreshold_ = value;
}

#if 0
void
MSLockMethod::setReferences( const wchar_t * dataClass, const wchar_t * xml )
{
    xmlDataClass_ = dataClass;
    xmlReferences_ = xml;
}

const wchar_t *
MSLockMethod::xmlDataClass() const
{
    return xmlDataClass_.c_str();
}

const wchar_t *
MSLockMethod::xmlReferences() const
{
    return xmlReferences_.c_str();
}
#endif
