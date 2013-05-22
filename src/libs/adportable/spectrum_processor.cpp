// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "spectrum_processor.hpp"
#include "array_wrapper.hpp"
#include <adportable/differential.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/variant.hpp>
#include <cmath>
#include <cstring> // for memset()
#include <stack>
#include <stdexcept>


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
        inline slope_counter(double _slope = 0, size_t _w = 3) : uc(0), dc(0), zc(0), bc(0), w(_w), n(0), slope(_slope) {}
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
spectrum_processor::moving_average( size_t nbrSamples, double * pY, const double * praw, size_t N )
{
    using adportable::differential;

    N |= 0x01; // make odd
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

double
spectrum_processor::area( const double * beg, const double * end, double base )
{
    double a = 0;
	for ( const double * p = beg; p != end; ++p ) {
		if ( *p > base )
			a += *p - base;
	}
    a += *end;
    return a;
}

spectrum_peakfinder::spectrum_peakfinder(double pw, double bw, WidthMethod wm ) : peakwidth_( pw )
                                                                                , atmz_( 0 )
                                                                                , baseline_width_( bw )
                                                                                , width_method_( wm ) 
{
}

namespace adportable { namespace peakfind {

        const size_t stack_size = 256;

        template< class T > class stack {
            T vec_[ stack_size ];
            T * bp_;
            inline const T * end() const  { return &vec_[ stack_size ]; }
            inline const T * begin() const { return &vec_[ 0 ]; }
        public:
            stack() : bp_( &vec_[ stack_size ] ) {   }
            inline bool empty() const                     { return bp_ == end(); }
            inline size_t size() const                    { return end() - bp_; }
            inline void push( const T& t )                { if ( bp_ <= begin() ) throw std::out_of_range("overflow"); *(--bp_) = t; }
            inline void pop()                             { if ( bp_ >= end() ) throw std::out_of_range("underflow");  ++bp_; }
            inline const T& top() const                   { return *bp_; }
            inline T& top()                               { return *bp_; }
            inline const T& operator [] ( int idx ) const { return bp_[ idx ]; }
            inline T& operator [] ( int idx )             { return bp_[ idx ]; }
        };

        enum event_type { None, Up, Down };

        struct counter {
            event_type type_;
            size_t bpos_;
            size_t tpos_;
            counter( size_t pos = 0, event_type type = None ) : type_( type ), bpos_( pos ), tpos_( pos ) {}
            counter( const counter& t ) : type_( t.type_ ), bpos_( t.bpos_ ), tpos_( t.tpos_ ) {}
            event_type type() const { return type_; }
            inline void operator ++ (int) { ++tpos_; }
            inline size_t distance() const { return tpos_ - bpos_; }
            inline void operator += ( const counter& t ) {
                assert ( t.type_ == type_ );
                tpos_ = t.tpos_;
            }
        };

        template<class T=counter> struct slope_state {

            stack< T > stack_;
            size_t width_;

            slope_state() : width_( 3 ) {}

            bool reduce( std::pair< T, T >& res ) {
                if ( stack_[1].type() == Down && stack_.size() >= 3 ) {
                    res.first = stack_[2];
                    res.second = stack_[1];
                    counter top = stack_.top();
                    stack_.pop();
                    stack_.pop();
                    stack_.pop();
                    stack_.push( top );
                    return true;
                }
                return false;
            }

            bool shift_reduce( const T& c ) {
                // delete count if too narrow width
                while ( stack_.top().distance() < width_ ) {
                    stack_.pop();
                    if ( stack_.empty() ) {
                        stack_.push( c );
                        return false;
                    }
                }

                // replace
                if ( stack_.top().type() == c.type() )  { // marge
                    stack_.top() += c;  // marge
                    //stack_.pop();
                    //stack_.push( c );  // replace
                }

                // if (Up - Down)|(Down - Up), should push counter and wait for next state
                if ( stack_.top().type() != c.type() )
                    stack_.push( c );

                return stack_.size() >= 3;
            };

            bool process_slope( const T& t ) {
                if ( stack_.empty() )
                    stack_.push( t );
                else if ( stack_.top().type() == t.type() )
                    stack_.top()++;
                else
                    return shift_reduce( t );
                return false;
            }
        };

    }
}

size_t
spectrum_peakfinder::operator()( size_t nbrSamples, const double *pX, const double * pY )
{
    array_wrapper<const double> px( pX, nbrSamples );
    array_wrapper<const double> py( pY, nbrSamples );
    
    pdebug_.resize( nbrSamples );
    memset( &pdebug_[0], 0, sizeof(double) * nbrSamples );

    array_wrapper<const double>::iterator it = std::upper_bound( px.begin(), px.end(), pX[0] + peakwidth_ );
    size_t w = std::distance( px.begin(), it );  // make odd
    size_t noise = 5; // assume LSB noise
    size_t N = ( w < 5 ) ? 5 : ( w > 25 ) ? 25 : w | 0x01;
    size_t NH = N / 2;

    double slope = double( noise ) / double( w );
    adportable::differential<double> diff( N, 1 );

    peakfind::slope_state<peakfind::counter> state;
    state.width_ = w / 8;
    if ( state.width_ < 3 )
        state.width_ = 3;

	averager base;
    for ( unsigned int x = NH; x < nbrSamples - NH; ++x ) {
        double d1 = diff( &pY[x] );
        bool reduce = false;
        if ( d1 >= slope )
            reduce = state.process_slope( peakfind::counter( x, peakfind::Up ) );
        else if ( d1 <= (-slope ) )
            reduce = state.process_slope( peakfind::counter( x, peakfind::Down ) );

        pdebug_[x] = d1;
        if ( reduce ) {

            std::pair< peakfind::counter, peakfind::counter > peak;

            while ( state.reduce( peak ) )
				results_.push_back( peakinfo( peak.first.bpos_, peak.second.tpos_, pY[ peak.first.bpos_ ] ) );

        }
    }

    if ( ! state.stack_.empty() ) {
        if ( state.stack_.top().type() == peakfind::Down )
            state.stack_.push( peakfind::counter( nbrSamples - 1, peakfind::None ) ); // dummy

        std::pair< peakfind::counter, peakfind::counter > peak;

        while ( state.reduce( peak ) )
            results_.push_back( peakinfo( peak.first.bpos_, peak.second.tpos_, pY[ peak.first.bpos_ ] ) );
    }

    return 0;
}

