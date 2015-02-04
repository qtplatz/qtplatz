/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

/** Savitzky-Golay filter coefficient genelrator

    References

    Hannibal H. Madden
    Comments on the Savitzky-Golay Convolution Method for Least-Squares Fit Smoothing and Differentiation of Digital Data
    Analytical Chemistry, (1978) 60(9) 1383-1986

    Original:
    Abraham Savitzky, A. and Marcel J. E. Golay, (1964). Analytical Chemistry (1964) 36(8) 1627-1639
    Smoothing and Differentiation of Data by Simplified Least Squares Procedures.

    Also an easy reference:
    http://en.wikipedia.org/wiki/Savitzky%E2%80%93Golay_filter#Tables_of_selected_convolution_coefficients    
**/

#include "sgfilter.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace adportable;

SGFilter::~SGFilter()
{
}

SGFilter::SGFilter( int m
                    , Filter filter
                    , PolynomialOrder order ) : filter_( filter )
                                              , order_( order )
                                              , m_( m )
{
    if ( filter_ == Smoothing ) {

        norm_ = Cubic::smoothing_coefficients( coefficients_, m_ );

    } else if ( filter_ == Derivative1 ) {

        if ( order == Quadratic )
            norm_ = Quadratic::derivative_1st_coefficients( coefficients_, m_ );            
        else
            norm_ = Cubic::derivative_1st_coefficients( coefficients_, m_ );

    } else if ( filter_ == Derivative2 ) {

        norm_ = Cubic::derivative_2nd_coefficients( coefficients_, m_ );

    } else

        throw std::invalid_argument( "Filter" );
}

SGFilter::SGFilter( const SGFilter& t ) : filter_( t.filter_ )
                                        , order_( t.order_ )
                                        , m_( t.m_ )
                                        , norm_( t.norm_ )
                                        , coefficients_( t.coefficients_ )
{
}

double
SGFilter::operator()( const double * y ) const
{
    return convolution( y, coefficients_.data(), norm_, m_ );
}

double
SGFilter::operator()( const int32_t * y ) const
{
    return convolution( y, coefficients_.data(), norm_, m_ );
}

double
SGFilter::operator()( const std::function< double(size_t) > fy, size_t cx ) const
{
    return convolution( fy, cx, coefficients_.data(), norm_, m_ );
}

int
SGFilter::m() const
{
    return m_;
}

const std::vector<double>&
SGFilter::coefficients() const
{
    return coefficients_;
}

// static
double
SGFilter::Quadratic::derivative_1st_coefficients( std::vector< double >& coeffs, int m )
{
    m |= 0x01; // should be odd
    int n = m / 2;

    coeffs.clear();    

    double denominator = 0;
    for ( int i = -n; i <= n; ++i )
        denominator += i * i;

    for ( int i = -n; i <= n; ++i ) {
        coeffs.push_back( i );
    }
    return denominator;
}


double
SGFilter::Cubic::smoothing_coefficients( std::vector< double >& coeffs, int m )
{
    m |= 0x01; // should be odd
    int n = m / 2;

    coeffs.clear();
    const double m2 = std::pow(m,2);

    double denominator = ( m * (m2 - 4) ) / 3.0;

    for ( int i = -n; i <= n; ++i ) {
        double numerator = (3*m2 - 7 - 20*std::pow(i,2))/4.0;
        coeffs.push_back( numerator / denominator );
        // std::cerr << "smooth[" << i << "]: " << numerator / denominator << "\t" << numerator << "/" << denominator << std::endl;
    }
    return 1.0;
}

double
SGFilter::Cubic::derivative_1st_coefficients( std::vector< double >& coeffs, int m )
{
    m |= 0x01; // should be odd
    int n = m / 2;
    coeffs.clear();

    const double m2 = m * m;
    const double m4 = m2 * m2;

    const double denominator = m * (m2 - 1)*(3*m4 - 39*m2 + 108)/15;
    for ( int i = -n; i <= n; ++i ) {
        double numerator = 5*(3*m4 - 18*m2 + 31)*i - 28*(3*m2 - 7) * std::pow(i, 3);
        coeffs.push_back( double(numerator) / double(denominator) );
        // std::cerr << "1st derivative[" << i << "]: " << numerator / denominator << "\t" << numerator << "/" << denominator << std::endl;
    }
    return 1.0;
}

double
SGFilter::Cubic::derivative_2nd_coefficients( std::vector< double >& coeffs, int m )
{
    m |= 0x01; // should be odd
    int n = m / 2;
    coeffs.clear();

    const double m2 = m * m;
    const double denominator = m2 * (m2 - 1)*(m2 - 4) / 15;

    for ( int i = -n; i <= n; ++i ) {
        const double numerator = 12 * m * std::pow(i, 2) - m * (m2 - 1);
        coeffs.push_back( double(numerator) / double(denominator) );
    }
    return 1.0;
}

// static
double
SGFilter::convolution( const double * y, const double * C, const double& norm, int m )
{
    double fxi = 0;
    y -= m / 2;
    for ( int i = 0; i < m; ++i )
        fxi += y[ i ] * C[ i ];
    return fxi / norm;
}

// static
double
SGFilter::convolution( const int32_t * y, const double * C, const double& norm, int m )
{
    double fxi = 0;
    y -= m / 2;
    for ( int i = 0; i < m; ++i )
        fxi += y[ i ] * C[ i ];
    return fxi / norm;
}

// static
double
SGFilter::convolution( const std::function< double( size_t ) > fy, size_t cx, const double * C, const double& norm, int m )
{
    double fxi = 0;
    if ( cx <(  m / 2 ) )
        return 0;
    cx -= m / 2;
    for ( int i = 0; i < m; ++i )
        fxi += fy( cx + i ) * C[ i ];
    return fxi / norm;
}

