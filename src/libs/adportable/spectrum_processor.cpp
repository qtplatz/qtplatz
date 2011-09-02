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

#include "spectrum_processor.hpp"
#include <adportable/differential.hpp>
#include <cmath>
#include <boost/smart_ptr.hpp>

#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>

using namespace adportable;

namespace adportable { // namespace internal {

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

    struct EvStart : boost::statechart::event< EvStart > {};
    struct EvStop : boost::statechart::event< EvStop > {};
    struct EvPeak : boost::statechart::event< EvPeak > {};
    struct Active;
    struct Stopped;
    struct PeakFinder : boost::statechart::state_machine< PeakFinder, Active > {};

    struct Active : boost::statechart::simple_state< Active, PeakFinder, Stopped > {
        typedef boost::statechart::transition< EvStop, Active > reactions;
        size_t spos_;
        size_t epos_;
        Active() : spos_(0), epos_(0) {}
    };

    struct Running : boost::statechart::simple_state< Running, Active > { };
    struct Stopped : boost::statechart::simple_state< Stopped, Active > { };

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

void
spectrum_processor::differentiation( size_t nbrSamples, double * pY, const double * intens, size_t N )
{
    using adportable::differential;

    const size_t Nhalf = N / 2;
    differential<double> diff( N );

    for ( unsigned int x = 0; x <= Nhalf; ++x )
        pY[ x ] = pY[ nbrSamples - 1 - x ] = 0;

    for ( unsigned int x = Nhalf; x < nbrSamples - Nhalf; ++x )
        pY[ x ] = diff( &intens[x] );
}

void
spectrum_processor::smoozing( size_t nbrSamples, double * pY, const double * praw, size_t N )
{
    using adportable::differential;

    double ax = 0;
    for ( size_t i = 0; i < nbrSamples; ++i ) {
        ax += praw[i];
        if ( i < (N/2) )
            pY[i] = praw[i] ;
        if ( i >= N ) {
            pY[i - (N/2)] = ax / double(N);
            ax -= praw[i - N];
        }
    }
}

size_t
spectrum_processor::findpeaks( size_t nbrSamples, const double * pY, double dbase, std::vector< std::pair<int, int> >& results, size_t N )
{
    using adportable::differential;

    results.clear();

    const size_t Nhalf = N / 2;
    differential<double> diff( N );

    double ss = 0.1;
    int uc = 0, dc = 0, zc = 0;
    const size_t width = 3;

    PeakFinder finder;
    finder.initiate();

    for ( unsigned int x = Nhalf; x < nbrSamples - Nhalf; ++x ) {
        double d1 = diff( &pY[x] );
        if ( d1 >= ss )
            finder.process_event( EvStart() );
        else if ( d1 < -ss )
            finder.process_event( EvPeak() );
        else
            finder.process_event( EvStop() );
    }
    return results.size();
}
