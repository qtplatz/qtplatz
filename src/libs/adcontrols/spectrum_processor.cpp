// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "spectrum_processor.h"
#include <adportable/differential.hpp>
#include <cmath>
#include <boost/smart_ptr.hpp>

using namespace adcontrols;
using namespace adcontrols::internal;

namespace adcontrols { namespace internal {

    static const double __norm5__ = 10;
    static const double __1st_derivative5__[] = { 0, 1, 2 };

    static const double __norm7__ = 28;
    static const double __1st_derivative7__[] = { 0, 1, 2, 3 };

    static const double __norm9__ = 60;
    static const double __1st_derivative9__[] = { 0, 1, 2, 3, 4 };

    template<typename T> static inline double convolute(const T * py) {
        double fxi;
        fxi  = 0; // __1st_derivative5__[0] * py[0];
        fxi += __1st_derivative5__[1] * ( -py[-1] + py[1] );
        fxi += __1st_derivative5__[2] * ( -py[-2] + py[2] );
        fxi = fxi / __norm5__;
        return fxi;  
    }
		
    struct slope_counter {
        size_t uc, dc, zc, bc;
        size_t w;
        size_t n;
        double slope;
        inline slope_counter(double _slope = 0, size_t _w = 3) : n(0), uc(0), dc(0), zc(0), bc(0), slope(_slope), w(_w) {}
        inline size_t operator()(const double& d1) {
            ++n;
            if ( d1 < -slope ) {
                ++dc;
                uc = zc = 0;
            } else if ( d1 > slope ) {
                ++uc;
                dc = zc = 0;
            } else {
                ++zc;
            }
            if ( (uc >= w) || ( dc >= w ) ) {
                bc = 0;
            } else {
                ++bc;
            }
            return bc;
        }
    };
		
    struct averager {
        int n;
        double ax;
		double sdd;
        averager() : n(0), ax(0), sdd(0) {}
        inline int operator()(const double& x) {
            ax += x;
			sdd += x * x;
            return ++n;
        }
        inline double average() const { return ax / n; }
        inline double rms() const { return std::sqrt( ( sdd / n ) - ( average() * average() ) ); }
    };


}
}


double
spectrum_processor::tic( unsigned int nbrSamples, const long * praw, double& dbase, double& rms )
{
    averager base;
    averager avgr;
    int cnt = 1;
    do {
        slope_counter counter(20.0);
        for ( unsigned int x = 2; x < nbrSamples - 2; ++x ) {
            avgr( praw[x] );
            if ( counter( convolute<long>( &praw[x] ) ) > 5 )
                base( praw[ x - 2 ] );
            else if ( counter.n > 5 )
                cnt++;
        }
    } while (0);
    dbase = base.average();
	rms = base.rms();
    return avgr.average() - dbase;
}

double
spectrum_processor::tic( unsigned int nbrSamples, const double * praw, double& dbase, double& rms )
{
    averager base;
    averager avgr;
    int cnt = 1;
    do {
        slope_counter counter(20.0);
        for ( unsigned int x = 2; x < nbrSamples - 2; ++x ) {
            avgr( praw[x] );
            if ( counter( convolute<double>( &praw[x] ) ) > 5 )
                base( praw[ x - 2 ] );
            else if ( counter.n > 5 )
                cnt++;
        }
    } while (0);
    dbase = base.average();
	rms = base.rms();
    return avgr.average() - dbase;
}

size_t
spectrum_processor::findpeaks( size_t nbrSamples, const double * praw, double dbase, std::vector< std::pair<int, int> >& results )
{
    using adportable::differential;

    results.clear();

    boost::scoped_array<double> pY( new double [ nbrSamples ] );
    double ax = 0;
    const size_t N = 5;
    for ( size_t i = 0; i < nbrSamples; ++i ) {
        ax += praw[i];
        if ( i < (N/2) )
            pY[i] = praw[i] ;
        if ( i >= N ) {
            pY[i - (N/2)] = ax / double(N);
            ax -= praw[i - N];
        }
    }

    differential<double> diff( 5 );
    
    do {
        double ss = 20.0;
        int uc = 0, dc = 0, zc = 0;
        const size_t width = 5;
        const size_t pwidth = 3;
        for ( unsigned int x = 2; x < nbrSamples - 2; ++x ) {
            double d1 = diff( &pY[x] );
            if ( d1 >= ss )
                ++uc;
            else if ( d1 < -ss )
                ++dc;
            else
                ++zc;
            if ( zc >= width )
                uc = dc = 0;
            if ( uc >= pwidth && dc >= pwidth ) {
                results.push_back( std::make_pair<int, int>( x - dc + uc, x ) );
                uc = 0;  // reduced
            }
        }
    } while (0);
    return results.size();
}
