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

#include "mscalibrateresult.hpp"
#include "msreference.hpp"
#include "msreferences.hpp"
#include "mscalibration.hpp"
#include "msassignedmass.hpp"
#include "msreference.hpp"

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>

#include <compiler/diagnostic_pop.h>

#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

using namespace adcontrols;

MSCalibrateResult::~MSCalibrateResult()
{
}

MSCalibrateResult::MSCalibrateResult() : tolerance_(0)
                                       , threshold_(0)
                                       , references_( new MSReferences )
                                       , calibration_( new MSCalibration ) 
                                       , assignedMasses_( new MSAssignedMasses ) 
                                       , mode_(0)
{
}

MSCalibrateResult::MSCalibrateResult( const MSCalibrateResult& t )
    : tolerance_( t.tolerance_ )
    , threshold_( t.threshold_ )
    , references_( new MSReferences( *t.references_ ) )
    , calibration_( new MSCalibration( *t.calibration_ ) )
    , assignedMasses_( new MSAssignedMasses( *t.assignedMasses_ ) )
    , mode_( t.mode_ )
    , description_( t.description_ )
{
}

const MSCalibrateResult&
MSCalibrateResult::operator = ( const MSCalibrateResult& t )
{
    tolerance_ = t.tolerance_;
    threshold_ = t.threshold_;
    references_.reset( new MSReferences( *t.references_ ) );
    calibration_.reset( new MSCalibration( *t.calibration_ ) );
    assignedMasses_.reset( new MSAssignedMasses( *t.assignedMasses_ ) );
    mode_ = t.mode_;
    description_ = t.description_;

	return *this;
}

double
MSCalibrateResult::tolerance() const
{
    return tolerance_;
}

void
MSCalibrateResult::tolerance( double v )
{
    tolerance_ = v;
}

double
MSCalibrateResult::threshold() const
{
    return threshold_;
}

void
MSCalibrateResult::threshold( double v )
{
    threshold_ = v;
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


MSAssignedMasses&
MSCalibrateResult::assignedMasses()
{
    return *assignedMasses_;
}

const MSAssignedMasses&
MSCalibrateResult::assignedMasses() const
{
    return *assignedMasses_;
}

void
MSCalibrateResult::assignedMasses( const MSAssignedMasses& t )
{
    *assignedMasses_ = t;
}

int
MSCalibrateResult::mode() const
{
    return mode_;
}

void
MSCalibrateResult::mode( int mode )
{
    mode_ = mode;
}

const std::wstring&
MSCalibrateResult::description() const
{
    return description_;
}

void
MSCalibrateResult::description( const std::wstring& text )
{
    description_ = text;
}

////////////////  static //////////////////
bool
MSCalibrateResult::archive( std::ostream& os, const MSCalibrateResult& t )
{
    portable_binary_oarchive ar( os );
    ar << t;
    return true;
}

bool
MSCalibrateResult::restore( std::istream& is, MSCalibrateResult& t )
{
    portable_binary_iarchive ar( is );
    ar >> t;
    return true;
}
