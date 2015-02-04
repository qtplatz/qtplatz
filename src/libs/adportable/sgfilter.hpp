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

#ifndef SGFILTER_HPP
#define SGFILTER_HPP

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

//#include <boost/numeric/ublas/matrix.hpp>
#include <vector>
#include <cstdint>
#include <functional>

namespace adportable {

    class SGFilter {
    public:
        enum Filter { Smoothing, Derivative1, Derivative2 };
        enum PolynomialOrder { Quadratic = 2, Cubic = 3 };
        ~SGFilter();
        SGFilter( int m = 5, Filter filter = Smoothing, PolynomialOrder = Cubic );
        SGFilter( const SGFilter& );

        double operator()( const double * y ) const;
        double operator()( const int32_t * y ) const;
        double operator()( const std::function< double(size_t) >, size_t cx ) const;
        double norm() const;
        int m() const;
        const std::vector<double>& coefficients() const;
        Filter filter() const;

        struct Quadratic {
            static double derivative_1st_coefficients( std::vector< double >& coeffs, int m );
        };

        struct Cubic { // Quadratic & Cubic
            static double smoothing_coefficients( std::vector< double >& coeffs, int m );
            static double derivative_1st_coefficients( std::vector< double >& coeffs, int m );
            static double derivative_2nd_coefficients( std::vector< double >& coeffs, int m );
        };

        static double convolution( const double * /* &y[i+m/2] */, const double * coefficients, const double& norm, int m );
        static double convolution( const int32_t * /* &y[i+m/2] */, const double * coefficients, const double& norm, int m );
        static double convolution( const std::function< double( size_t ) >, size_t cx, const double * coefficients, const double& norm, int m );
        
    private:
        Filter filter_;
        PolynomialOrder order_;
        int m_;
        double norm_;
        std::vector< double > coefficients_;
    };

}

#endif // SGFILTER_HPP
