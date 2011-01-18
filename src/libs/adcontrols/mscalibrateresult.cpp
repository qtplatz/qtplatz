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

#include "mscalibrateresult.h"
#include "msreference.h"
#include "msreferences.h"
#include "mscalibration.h"

using namespace adcontrols;

MSCalibrateResult::~MSCalibrateResult()
{
}

MSCalibrateResult::MSCalibrateResult() : references_( new MSReferences )
                                       , calibration_( new MSCalibration ) 
{
}

MSCalibrateResult::MSCalibrateResult( const MSCalibrateResult& t )
: references_( new MSReferences( *t.references_ ) )
, calibration_( new MSCalibration( *t.calibration_ ) )
{
}


const MSReferences&
MSCalibrateResult::references() const
{
    return *references_;
}

MSReferences&
MSCalibrateResult::references()
{
    return *references_;
}

void
MSCalibrateResult::references( const MSReferences& t )
{
    *references_ = t;
}

const MSCalibration&
MSCalibrateResult::calibration() const
{
    return *calibration_;
}

MSCalibration&
MSCalibrateResult::calibration()
{
    return *calibration_;
}

void
MSCalibrateResult::calibration( const MSCalibration& t )
{
    *calibration_ = t;
}

