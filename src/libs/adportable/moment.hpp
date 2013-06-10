// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include <math.h>
#include <vector>
#include <assert.h>

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

    template<class Fx> class Moment {
        Fx& fx_;
    public:
        Moment( Fx& f ) : fx_(f) {}

        double width( const double * py, double threshold, size_t spos, size_t tpos, size_t epos ) {
            long xL = left_bound<double>( py, threshold, tpos, spos );
            long xR = right_bound<double>( py, threshold, tpos, epos );
            double Xl = left_intersection( py, xL, threshold );
            double Xr = right_intersection( py, xR, threshold );
            return Xr - Xl;
        }

        double centerX( const double * py, double threshold, size_t spos, size_t tpos, size_t epos ) {
            long xL = left_bound<double>( py, threshold, tpos, spos );
            long xR = right_bound<double>( py, threshold, tpos, epos );
            double Xl = left_intersection( py, xL, threshold );
            double Xr = right_intersection( py, xR, threshold );

            double x0 = fx_( xL - 1 );
            double ma = 0;
            double ta = 0;
            double baseH = threshold;

            {
                // left triangle
                //
                //  90deg rotated triangle (h == time axis)
                //     /|
                //    / | h   A  = (1/2) a * h;
                //   /__|     Gy = (1/3) * h
                //    a
                double h = fx_( xL ) - Xl;
                double a = py[xL] - baseH;
                double area = h * a / 2;
                double Gy = h / 3;
                double cx = fx_( xL ) - Gy - x0;
                ta += area;
                ma += area * cx;
            }
            {
                // right triangle
                double h = Xr - fx_( xR );
                double a = py[xR] - baseH;
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
                double a = py[x + 1] - baseH;
                double c = py[x] - baseH;
                double Gy = (h * (a + 2 * c)) / (3 * (a + c));
                double area = (c + a) * h / 2;
                ta += area;
                double cx = ( fx_( x + 1 ) - Gy) - x0;
                ma += area * cx;
            };
            double xc = ma / ta + x0;
            return xc;
        };

    private:
        double left_intersection( const double * py, size_t x, double threshold ) const {
            if (py[x - 1] >= threshold)
                return fx_( x - 1 );
            if (py[x] < threshold)  // should not be here, it's a bug
                return fx_( x );
            return fx_( x - 1 ) + (fx_( x ) - fx_( x - 1 ) ) * (threshold - py[x - 1]) / (py[ x ] - py[ x - 1 ]);
        };

        double right_intersection( const double * py, size_t x, double threshold ) const {
            if ( py[x + 1] >= threshold )
                return fx_( x + 1 );
            if ( py[x] < threshold )  // should not be here, it's a bug
                return fx_( x );
            return fx_( x ) + (double)( fx_(x + 1) - fx_(x)) * (py[x] - threshold) / (double)(py[x] - py[x + 1]);
        };

        template<typename T> static size_t left_bound( const T* py, const T threshold, size_t tpos, size_t spos ) {
            size_t x = tpos - 1;
            while ((py[x] >= threshold) && x > spos)
                --x;
            return x;
        };

        template<typename T> static size_t right_bound( const T* py, const T threshold, size_t tpos, size_t epos ) { 
            size_t x = tpos + 1;
            while ((py[x] >= threshold) && x < epos)
                ++x;
            return x;
        };
    };
}
