// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#pragma once

#include <cmath>
#include <functional>

namespace adportable {

    struct timeFunctor {
        timeFunctor( size_t startDelay, double sampInterval ) : interval(sampInterval), delay(startDelay) {}
        double operator ()( int pos ) { return ( delay + pos ) * interval; }
        double interval;
        size_t delay;
    };

    struct massArrayFunctor {
        massArrayFunctor( const double * masses, size_t nbrSamples ) : masses_( masses ), nbrSamples_( nbrSamples ) { }
        double operator ()( int pos ) {
            if ( pos >= 0 && pos < int( nbrSamples_ ) )
                return masses_[ pos ];
            return 0;
        }
        const double * masses_;
        size_t nbrSamples_;
    };

    /////////////////////////////////////////////////////

    class Moment {
        std::function< double( int pos ) > fx_;
        double Xl;
        double Xr;
    public:
        Moment( std::function< double( int pos ) > fx ) : fx_( fx ) {}

        inline double xLeft() const { return Xl; }
        inline double xRight() const { return Xr; }

        double width( std::function< double( int pos ) > fy, double threshold, uint32_t spos, uint32_t tpos, size_t epos ) {
            int xL = left_bound<double>( fy, threshold, tpos, spos );
            int xR = right_bound<double>( fy, threshold, tpos, epos );
            Xl = left_intersection( fy, xL, spos, threshold );
            Xr = right_intersection( fy, xR, epos, threshold );
            return Xr - Xl;
        }
        double width( const double * py, double threshold, uint32_t spos, uint32_t tpos, size_t epos ) {
            return width( [&]( int pos ){ return py[ pos ]; }, threshold, spos, tpos, epos );
        }

        double centreX( std::function< double( int pos ) > fy, double threshold, uint32_t spos, uint32_t tpos, size_t epos ) {
            int xL = left_bound<double>( fy, threshold, tpos, spos );
            int xR = right_bound<double>( fy, threshold, tpos, epos );
            Xl = left_intersection( fy, xL, spos, threshold );
            Xr = right_intersection( fy, xR, epos, threshold );

            double x0 = fx_( xL - 1 );
            double ma = 0;
            double ta = 0;
            double baseH = threshold;

            {
                // left triangle
                //
                //  90deg clockwise rotated triangle (h == time axis)
                //     /|
                //    / | h   A  = (1/2) a * h;
                //   /__|     Gy = (1/3) * h
                //    a
                double h = fx_( xL ) - Xl;
                double a = fy( xL ) - baseH;
                double area = h * a / 2;
                double Gy = h / 3;
                double cx = fx_( xL ) - Gy - x0;
                ta += area;
                ma += area * cx;
            }
            {
                // right triangle
                double h = Xr - fx_( xR );
                double a = fy( xR ) - baseH;
                double area = h * a / 2;
                double Gy = h / 3;
                double cx = fx_( xR ) + Gy - x0;
                ta += area;
                ma += area * cx;
            }
            //ta = aL + aR;
            //ma = mL + mR;

            for (long x = xL; x < xR; ++x) {
                // 90 deg rotated trapezium
                //         c
                //      ________
                //     /       |           A  = (1/2) * (a + c) * h;
                //  d /   Gy   | b = h           h(a + 2c)
                //   /_________|           Gy = ------------
                //        a                      3(a + c)
                //
                double h = fx_( x + 1 ) - fx_( x );
                double a = fy(x + 1 ) - baseH;
                double c = fy( x ) - baseH;
                double Gy = (h * (a + 2 * c)) / (3 * (a + c));
                double area = (c + a) * h / 2;
                ta += area;
                double cx = ( fx_( x + 1 ) - Gy) - x0;
                ma += area * cx;
            };
            double xc = ma / ta + x0;
            return xc;
        };

        double centreX( const double * py, double threshold, uint32_t spos, uint32_t tpos, size_t epos ) {
            return centreX( [&]( int pos ){ return py[ pos ]; }, threshold, spos, tpos, epos );
        }

    private:
        double left_intersection( std::function< double( int pos ) > fy, uint32_t x, uint32_t llimit, double threshold ) const {
            if ( x <= llimit )
                return fx_( x ); // no more data on left
            // interporation between x and x - 1
            return fx_( x - 1 ) + ( fx_( x ) - fx_( x - 1 ) ) * ( threshold - fy(x - 1) ) / ( fy( x ) - fy( x - 1 ) );
        };

        double right_intersection( std::function< double( int pos ) > fy, uint32_t x, uint32_t rlimit, double threshold ) const {
            if ( x >= rlimit )  // no more data on right
                return fx_( x );
            // interporation between x and x + 1
            return fx_( x ) + ( fx_( x + 1 ) - fx_( x ) ) * ( fy( x ) - threshold ) / ( fy( x ) - fy( x + 1 ) );
        };

        //-----
        template<typename T> static int left_bound( std::function< double( int pos ) > fy, const T threshold, size_t tpos, size_t spos ) {
            size_t x = tpos - 1;
            while ( ( x >= spos ) && ( fy(x) > threshold ) )
                --x;
            return int(++x);
        };

        template<typename T> static int right_bound( std::function< double( int pos ) > fy, const T threshold, size_t tpos, size_t epos ) {
            size_t x = tpos + 1;
            while ( ( x <= epos ) && ( fy(x) > threshold ) )
                ++x;
            return int(--x);
        };

    };
}
