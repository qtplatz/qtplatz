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
#include "massspectrometer.hpp"
#include "metric/prefix.hpp"
#include <sstream>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

using namespace adcontrols;
using namespace adcontrols::massspectrometer;

MSCalibration::MSCalibration() : t0_method_( LINEAR_TO_SQRT_M )
                               , time_prefix_( adcontrols::metric::base )
{
    init();
}

MSCalibration::MSCalibration( const MSCalibration& t ) : calibDate_( t.calibDate_ )
                                                       , calibId_( t.calibId_ )
                                                       , coeffs_( t.coeffs_ )  
                                                       , t0_coeffs_( t.t0_coeffs_ )
                                                       , t0_method_( t.t0_method_ )
                                                       , time_prefix_( t.time_prefix_ )
                                                       , time_method_( t.time_method_ )
{
}

MSCalibration::MSCalibration( const std::vector<double>& v
                              , metric::prefix pfx ) : coeffs_( v )
                                                     , time_prefix_( pfx )
                                                     , t0_method_( LINEAR_TO_SQRT_M )
                                                     , time_method_( NOTHING )
{
    init();
}

MSCalibration::MSCalibration( const std::vector<double>& coeffs
                              , metric::prefix time_prefix
                              , const std::vector<double>& t0Coeff
                              , TIME_METHOD time_method ) : coeffs_( coeffs )
                                                        , t0_coeffs_( t0Coeff )
                                                        , time_prefix_( time_prefix )
                                                        , t0_method_( LINEAR_TO_SQRT_M ) 
                                                        , time_method_( time_method )
{
    init();
}

void
MSCalibration::init()
{
    boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time();
    calibDate_ = boost::lexical_cast< std::string >( pt );

	const boost::uuids::uuid uuid = boost::uuids::random_generator()();
    calibId_ = boost::lexical_cast< std::wstring >( uuid );
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

const std::vector< double >&
MSCalibration::t0_coeffs() const
{
    return t0_coeffs_;
}

void
MSCalibration::t0_coeffs( const std::vector<double>& v )
{
    t0_coeffs_ = v;
}

void
MSCalibration::t0_method( T0_METHOD value )
{
    t0_method_ = value;
}

MSCalibration::T0_METHOD
MSCalibration::t0_method() const
{
    return t0_method_;
}

void
MSCalibration::time_method( TIME_METHOD value )
{
    time_method_ = value;
}

MSCalibration::TIME_METHOD
MSCalibration::time_method() const
{
    return time_method_;
}

double
MSCalibration::compute_mass( double time ) const
{
    double sqrt = compute( coeffs_, time );
    if ( sqrt > 0 )
        return sqrt * sqrt;

    return 0;
}

double
MSCalibration::compute_mass( double time, const ScanLaw& law, int mode ) const
{
    if ( time_method_ == MULTITURN_NORMALIZED ) {
        double t0 = 0;
        if ( t0_coeffs_.empty() )
            return compute_mass( scale_to( time_prefix_, time - law.getTime(0, mode) ) / law.fLength( mode ) );

        double mass = law.getMass( time, mode );
		for ( int i = 0; i < 2; ++i ) {
            t0 = metric::scale_to_base( compute( t0_coeffs_, std::sqrt( mass ) ), time_prefix_ );
			double T  = scale_to( time_prefix_, time - t0 );
			mass = compute_mass( T / law.fLength( mode ) );
		}
        return mass;
    } else {
        double T = metric::scale_to( time_prefix_, time );
        if ( ! t0_coeffs_.empty() )
            T -= t0_coeffs_[0];
        return compute_mass( T );
    }
    return 0;
}


// static
double
MSCalibration::compute( const std::vector<double>& v, double t )
{
	int n = 0;
	double sqmz = 0;
	for ( auto d: v )
		sqmz += d * std::pow( t, n++ );
	return sqmz;
}

void
MSCalibration::time_prefix( metric::prefix pfx )
{
    time_prefix_ = pfx;
}

metric::prefix
MSCalibration::time_prefix() const
{
    return time_prefix_;
}

std::string
MSCalibration::formulaText( bool ritchText )
{
    std::ostringstream o;

    if ( coeffs_.empty() )
        return "";

    if ( ritchText ) {
        o << "&radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span> = "
          << boost::format( "%.8le" ) % coeffs_[ 0 ];
        for ( size_t i = 1; i < coeffs_.size(); ++i )
            o << boost::format( " + %.8le &times; t<sup>%d</sup>" ) % coeffs_[ i ] % i;
    } else {
        o << "sqrt(m/z) = "
          << boost::format( "%.8le" ) % coeffs_[ 0 ];
        for ( size_t i = 1; i < coeffs_.size(); ++i )
            o << boost::format( " + %.8le * t^%d" ) % coeffs_[ i ] % i;
    }
    return o.str();
}
