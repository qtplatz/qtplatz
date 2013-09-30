// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "mscalibration.hpp"

using namespace adcontrols;

// all newly created calibration to be used to microseconds
// old data still keep seconds for time
MSCalibration::MSCalibration() : time_prefix_( metric::micro ) 
{
}

MSCalibration::MSCalibration( const MSCalibration& t ) : calibDate_( t.calibDate_ )
                                                       , calibId_( t.calibId_ )
                                                       , coeffs_( t.coeffs_ )
                                                       , time_prefix_( t.time_prefix_ )
{
}

MSCalibration::MSCalibration( const std::vector<double>& v, metric::prefix prefix ) : coeffs_( v )
                                                                                    , time_prefix_( prefix )
{
}

metric::prefix
MSCalibration::time_prefix() const
{
    return time_prefix_;
}

const std::string&
MSCalibration::date() const
{
    return calibDate_;
}

void
MSCalibration::date( const std::string& date )
{
    calibDate_ = date;
}

const std::wstring&
MSCalibration::calibId() const
{
    return calibId_;
}

void
MSCalibration::calibId( const std::wstring& calibId )
{
    calibId_ = calibId;
}

const std::vector< double >&
MSCalibration::coeffs() const
{
    return coeffs_;
}

void
MSCalibration::coeffs( const std::vector<double>& v, metric::prefix prefix )
{
    coeffs_ = v;
    time_prefix_ = prefix;
}

double
MSCalibration::compute_mass( double time, metric::prefix prefix ) const
{
    double t = metric_prefix( time, prefix, time_prefix_ ); // adjust time value to be equivalent

	int n = 0;
	double sqrt = 0;

	for ( auto d: coeffs_ )
		sqrt += d * std::pow( t, n++ );

    if ( sqrt > 0 )
        return sqrt * sqrt;

    return 0;
}

// static
// double
// MSCalibration::compute( const std::vector<double>& v, double t )
// {
// 	int n = 0;
// 	double sqmz = 0;
// 	for ( auto d: v )
// 		sqmz += d * std::pow( t, n++ );
// 	return sqmz;
// }
