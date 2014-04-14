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

#include "sgfilter.hpp"

SGFilter::~SGFilter()
{
}

SGFilter::SGFilter( int m, Filter filter ) : m_( m )
                                           , filter_( filter )
{
    if ( filter_ == Smoothing )
        norm_ = Cubic::smoothing_coefficients( coefficients_, m_ );
    else if ( filter_ == Derivative1 )
        norm_ = Cubic::derivative_1st_coefficients( coefficients_, m_ );
    else
        throw std::bad_cast();
}

SGFilter::SGFilter( const SGFilter& t ) : m_( t.m_ )
                                        , norm_( t.norm_ )
                                        , coefficients_( t.coefficients_ )
{
}

double
SGFilter::Cubic::smoothing_coefficients( std::vector< double >& coeffs, int m )
{
    m |= 0x01; // should be odd
    int n = m / 2;
    double cd = ( ( m * (std::pow(m,2) - 4 ) ) / 3 );
    
    coeffs.resize( m );
    const double m2 = std::pow( m, 2 );
    
    int k = 0;
    for ( int i = -n; i <= n; ++i ) {
        double cn = 3 * ( 3*m2 + 3 * m - 1 - 5*(double(i) * i);
        double cn = ( ( 3 * m2 - 7 - 20 * std::pow(double(i), 2) ) / 4.0 );
        coeffs[k++] = cn / cd;
    }
}

double
SGFilter::Cubic::derivative_1st_coefficients( std::vector< double >& coeffs, int m )
{
    m |= 0x01; // should be odd
    int n = m / 2;
    coeffs.resize( m );

    const double m2 = std::pow(m, 2);
    const double m4 = std::pow(m, 4);

    double cd = m * (m2 - 1) * (3 * m4 - 39.0 * m2 + 108) / 15.0;
    int k = 0;
    for ( int i = -n; i <= n; ++i ) {
        double cn = ( 5 * ( 3 * m4 - 18 * m2 + 31 ) * i ) - ( 28 * (3 * m2 - 7) * std::pow(double(i),3) );
        coeffs[k++] = cn / cd;
    }
}

double
SGFilter::Cubic::derivative_2nd_coefficients( std::vector< double >& coeffs, int m )
{
    m |= 0x01; // should be odd
    int n = m / 2;
    coeffs.resize( m );

    const double m2 = std::pow(m, 2);

    double cd = m * (m2 - 1.0) * (m2 - 4.0) / 15.0;
    int k = 0;
    for ( int i = -n; i <= n; ++i ) {
        double cn = 12.0 * m * (double(i) * i) - m * (m2 - 1);
        coeffs[k++] = cn / cd;
    }
}

// static
double
SGFilter::convolution( const double * y, const double * C, const double& norm, int m ) const
{
    double fxi = 0;
    y -= m / 2;
    for ( int i = 0; i < m; ++i )
        fxi += y[ i ] * C[ i ];
    return fxi / norm;
}
