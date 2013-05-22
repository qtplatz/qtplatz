// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

MSCalibration::MSCalibration()
{
}

MSCalibration::MSCalibration( const MSCalibration& t ) : calibDate_( t.calibDate_ )
                                                       , calibId_( t.calibId_ )
                                                       , coeffs_( t.coeffs_ )  
{
}

MSCalibration::MSCalibration( const std::vector<double>& v ) : coeffs_( v )
{
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
MSCalibration::coeffs( const std::vector<double>& v )
{
    coeffs_ = v;
}

// static
double
MSCalibration::compute( const std::vector<double>& v, double t )
{
    if ( v.empty() )
        return 0;

    switch ( v.size() ) {
    case 1:
        return v[0];
    case 2:
        return v[0] + t * v[1];
    case 3:
        return v[0] + t * v[1] + t * ( v[2] * v[2] );
    default:
        break;
    };

    std::vector<double>::const_iterator p = v.begin();
    double r = *p++;
    int n = 1;
    while ( p != v.end() )
        r += t * std::pow( *p++, n++ );
    return r;
}