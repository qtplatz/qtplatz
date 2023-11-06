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

#include "peakasymmetry.hpp"
#include <adportable/json_helper.hpp>
#include <adportable/json/extract.hpp>
#include <boost/json.hpp>

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
PeakAsymmetry::setAsymmetry( double value )
{
    peakAsymmetry_ = value;
}

void
PeakAsymmetry::setBoundary( double left, double right )
{
    peakAsymmetryStartTime_ = left;
    peakAsymmetryEndTime_ = right;
}

double
PeakAsymmetry::startTime() const
{
    return peakAsymmetryStartTime_;
}

double
PeakAsymmetry::endTime() const
{
    return peakAsymmetryEndTime_;
}

namespace adcontrols {

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const PeakAsymmetry& t )
    {
        jv = {
            { "Asymmetry",            t.peakAsymmetry_   }
            ,{ "AsymmetryStartTime",  t.peakAsymmetryStartTime_ }
            ,{ "AsymmetryEndTime",    t.peakAsymmetryEndTime_ }
        };
    }

    PeakAsymmetry
    tag_invoke( const boost::json::value_to_tag< PeakAsymmetry >&, const boost::json::value& jv )
    {
        PeakAsymmetry _;
        using namespace adportable::json;

        if ( jv.is_object() ) {
            auto obj = jv.as_object();
            extract( obj,     _.peakAsymmetry_,        "Asymmetry"        );
            extract( obj,     _.peakAsymmetryStartTime_,   "AsymmetryStartTime"        );
            extract( obj,     _.peakAsymmetryEndTime_,     "AsymmetryStartTime"        );
        }
        return _;
    }
}
