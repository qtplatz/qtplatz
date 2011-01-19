//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "mscalibration.h"

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