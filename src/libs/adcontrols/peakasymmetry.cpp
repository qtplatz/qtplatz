// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "peakasymmetry.hpp"

using namespace adcontrols;

PeakAsymmetry::PeakAsymmetry() : peakAsymmetry_(0)
                               , peakAsymmetryStartTime_(0)
                               , peakAsymmetryEndTime_(0)
{
}

PeakAsymmetry::PeakAsymmetry( const PeakAsymmetry& t ) : peakAsymmetry_( t.peakAsymmetry_ )
                                                       , peakAsymmetryStartTime_( t.peakAsymmetryStartTime_ )
                                                       , peakAsymmetryEndTime_( t.peakAsymmetryEndTime_ )
{
}

double
PeakAsymmetry::asymmetry() const
{
    return peakAsymmetry_;
}

void
PeakAsymmetry::asymmetry( double value )
{
    peakAsymmetry_ = value;
}

double
PeakAsymmetry::startTime() const
{
    return peakAsymmetryStartTime_;
}

void
PeakAsymmetry::startTime( double value )
{
    peakAsymmetryStartTime_ = value;
}

double
PeakAsymmetry::endTime() const
{
    return peakAsymmetryEndTime_;
}

void
PeakAsymmetry::endTime( double value )
{
    peakAsymmetryEndTime_ = value;
}
