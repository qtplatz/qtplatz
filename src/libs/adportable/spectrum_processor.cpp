/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>
#include "spectrum_processor.hpp"
#include "array_wrapper.hpp"
#include "polfit.hpp"
#include <boost/variant.hpp>
#include <compiler/diagnostic_pop.h>
#include "moment.hpp"
#include "sgfilter.hpp"

#include <cmath>
#include <cstring> // for memset()
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <fstream>
#if defined _DEBUG || defined DEBUG
# include <iomanip>
#endif

using namespace adportable;

namespace adportable {

    static const double __norm5__ = 10;
    static const double __1st_derivative5__[] = { 0, 1, 2 };

    template<typename T> static inline double convolute(const T * py) {
        double fxi;
        fxi  = 0; // __1st_derivative5__[0] * py[0];
        fxi += __1st_derivative5__[1] * ( -py[-1] + py[1] );
        fxi += __1st_derivative5__[2] * ( -py[-2] + py[2] );
        fxi = fxi / __norm5__;
        return fxi;  
    }
    template<typename _fy> static inline double convolute( const _fy fy, size_t idx ) {
        double fxi;
        fxi  = 0; // __1st_derivative5__[0] * py[0];
        fxi += __1st_derivative5__[1] * ( -fy( idx - 1 ) + fy( idx + 1 ) );
        fxi += __1st_derivative5__[2] * ( -fy( idx - 2 ) + fy( idx + 2 ) );
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
                if ( zc >= w * 2 )
                    uc = dc = 0;
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
        inline double average() const { return n ? ( ax / n ) : ax; }
        inline double rms() const { return n ? ( std::sqrt( ( sdd / n ) - ( average() * average() ) ) ) : 0 ; }
    };

    template<typename T> struct areaCalculator {

        static double area( const spectrum_processor::areaFraction& frac, double baseH, const T* pData, size_t size ) {

            //assert( frac.lFrac >= 0.0 && frac.lFrac <= 1.0 );
            //assert( frac.uFrac >= 0.0 && frac.uFrac <= 1.0 );
            //if ( frac.lPos == frac.uPos )
            //    return double( pData[ frac.lPos ] ) - baseH;

            double ax = 0;
            for ( size_t i = frac.lPos; i <= frac.uPos; ++i ) {
                double h = pData[ i ] - baseH;
                ax += h;
            }
            double a_trapesium = 0;
            if ( frac.lPos > 0 ) { // if one before data point exist
                // Left trapesium area
                double y0 = std::min( pData[ frac.lPos ], pData[ frac.lPos - 1 ] ) - baseH;
                double y1 = std::max( pData[ frac.lPos ], pData[ frac.lPos - 1 ] ) - baseH;
                double y = (y1 - y0) * (1.0 - frac.lFrac) + y0;
                double a = (y + y1) * frac.lFrac / 2.0;
                ax += a;
                a_trapesium = a;
            }

            if ( frac.uPos < size ) { // if following data point exist
                // Right trapesium area
                double y0 = std::max( pData[ frac.uPos ], pData[ frac.uPos + 1 ] ) - baseH;
                double y1 = std::min( pData[ frac.uPos ], pData[ frac.uPos + 1 ] ) - baseH;
                double y = y0 - (y0 - y1) * frac.uFrac;
                double a = (y + y0) * frac.uFrac / 2.0;
                ax += a;
                a_trapesium += a;
            }
#if defined _DEBUG || defined DEBUG
            double portion = ax / a_trapesium;
            (void)portion;
#endif
            return ax; // x-axis is based on bin number (assume equally spaced data array)
        }
    };

    struct tic_calculator {

        template<typename T> double operator () ( size_t nbrSamples, const T * praw, double& dbase, double& rms, size_t N ) {

            if ( nbrSamples < N )
                return 0.0;

            averager base;
            int cnt = 1;
            do {
                slope_counter counter(20.0);
                for ( size_t x = (N/2); x < nbrSamples - (N/2); ++x ) {
                    if ( counter( convolute<T>( &praw[x] ) ) > N )
                        base( praw[ x - (N/2) ] );
                    else if ( counter.n > N )
                        cnt++;
                }
            } while (0);

            dbase = base.average();
            rms = base.rms();
            double ax = 0;
            for ( size_t i = 0; i < nbrSamples; ++i )
                ax += praw[ i ] - dbase;
            return ax; //avgr.average() - dbase;
        }

        template< typename _fy > double
        operator () ( _fy fy, size_t beg, size_t end, double& dbase, double& rms, size_t N ) {
            averager base;
            int cnt = 1;
            do {
                slope_counter counter(20.0);
                for ( size_t x = beg + (N/2); x < end - (N/2); ++x ) {
                    if ( counter( convolute( fy, x ) ) > N )
                        base( fy( x - (N/2) ) );
                    else if ( counter.n > N )
                        cnt++;
                }
            } while (0);
            
            dbase = base.average();
            rms = base.rms();
            double ax = 0;
            for ( size_t x = beg; x < end; ++x )
                ax += fy(x) - dbase;
            return ax;
        }

    };

}

double
spectrum_processor::tic( size_t nbrSamples, const int8_t * praw, double& dbase, double& rms, size_t N )
{
    return tic_calculator()( nbrSamples, praw, dbase, rms, N );
}

double
spectrum_processor::tic( size_t nbrSamples, const int16_t * praw, double& dbase, double& rms, size_t N )
{
    return tic_calculator()( nbrSamples, praw, dbase, rms, N );
}

double
spectrum_processor::tic( size_t nbrSamples, const int32_t * praw, double& dbase, double& rms, size_t N )
{
    return tic_calculator()( nbrSamples, praw, dbase, rms, N );
}

double
spectrum_processor::tic( size_t nbrSamples, const int64_t * praw, double& dbase, double& rms, size_t N )
{
    return tic_calculator()( nbrSamples, praw, dbase, rms, N );
}

double
spectrum_processor::tic( size_t nbrSamples, const double * praw, double& dbase, double& rms, size_t N )
{
    return tic_calculator()( nbrSamples, praw, dbase, rms, N );
}

void
spectrum_processor::differentiation( size_t nbrSamples, double * pY, const double * intens, size_t m )
{
    SGFilter d1( int(m), SGFilter::Derivative1 );

    // fill head & tail
    for ( size_t x = 0; x < (m/2); ++x ) {
        pY[ x ] = intens[ x ];
        pY[ nbrSamples - 1 - x ] = intens[ nbrSamples - 1 - x ];
    }

    for ( size_t x = (m/2); x < nbrSamples - (m/2); ++x )
        pY[ x ] = d1( &intens[x] );

}

void
spectrum_processor::moving_average( size_t nbrSamples, double * pY, const double * praw, size_t m )
{
    m |= 0x01; // make odd
    
    double ax = 0;
    for ( size_t i = 0; i < nbrSamples; ++i ) {
        ax += praw[i];
        if ( i < (m/2) )
            pY[i] = praw[i] ;
        if ( i >= m ) {
            pY[i - (m/2) - 1] = ax / double(m);
            ax -= praw[i - m];
		}
    }
	for ( size_t i = nbrSamples - (m/2) - 1; i < nbrSamples; ++i )
		pY[i] = ax;
}

/** \brief simple peak area calculation.  
 *
 *  make sum for data between beg to end - 1.
 */
double
spectrum_processor::area( const double * beg, const double * end, double base )
{
    double a = 0;
	for ( const double * p = beg; p != end; ++p ) {  
		if ( *p > base )
			a += *p - base;
	}
    return a;
}

spectrum_peakfinder::spectrum_peakfinder(double pw
                                         , double /* bw */
                                         , WidthMethod wm ) : peakwidth_( pw )
                                                            , atmz_( 0 )
                                                            , width_method_( wm ) 
{
}

namespace adportable { namespace peakfind {

        const size_t stack_size = 64;

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

            slope_state( size_t w = 5 ) : width_( ( w < 3 ? 3 : w ) ) {}

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
                while ( stack_.top().distance() < width_ ) {
                    // && stack_.top().type() == peakfind::Down ) { // discard narrow 'down' state
                    stack_.pop();
                    if ( stack_.empty() ) {
                        stack_.push( c );
                        return false;
                    }
                }

                if ( stack_.top().type() == c.type() )  // marge
                    stack_.top() += c;  // marge

                // if (Up - Down)|(Down - Up), should push counter and wait for next state
                if ( stack_.top().type() != c.type() )
                    stack_.push( c );

                return stack_.size() >= 3;
            };

            bool process_slope( const T& t ) {
                if ( stack_.empty() ) {
                    stack_.push( t ); // Up|Down
                    return false;
                } else if ( stack_.top().type() == t.type() ) {
                    stack_.top()++;
                    return false;
                } else
                    return shift_reduce( t );
            }
        };

    }
}

size_t
spectrum_peakfinder::operator()( size_t nbrSamples, const double * pX, const double * pY )
{
    if ( pX == 0 || pY == 0 )
        return 0;
    array_wrapper<const double> px( pX, nbrSamples );
    array_wrapper<const double> py( pY, nbrSamples );
    
    size_t w = 7; // std::distance( xIt, it );  // make odd
    size_t atmz = 0;
    if ( width_method_ == TOF ) {
		if ( ( pX[0] < atmz_ ) && ( atmz_ + peakwidth_ < pX[ nbrSamples - 1 ] ) ) {
			array_wrapper<const double>::iterator xIt1 = std::upper_bound( px.begin(), px.end(), atmz_ );
			array_wrapper<const double>::iterator xIt2 = std::upper_bound( xIt1, px.end(), atmz_ + peakwidth_ );
			w = std::distance( xIt1, xIt2 );
            atmz = size_t( atmz_ * 4 );
		} else {
			array_wrapper<const double>::iterator xIt1 = std::upper_bound( px.begin(), px.end(), pX[ nbrSamples / 4 ] );
			array_wrapper<const double>::iterator xIt2 = std::upper_bound( xIt1, px.end(), *xIt1 + peakwidth_ );
			w = std::distance( xIt1, xIt2 );
		}
    }
    double dbase(0), rms(0);
    spectrum_processor::tic( nbrSamples, pY, dbase, rms );

    // int noise = int(rms); //5; // assume LSB noise
    int m = ( w < 5 ) ? 5 : int(w) | 0x01; // ( w > 25 ) ? 25 : w | 0x01;
    int NH = m / 2;

    double slope = double( rms ) / double( w * 8 );
    SGFilter diff( m, SGFilter::Derivative1, SGFilter::Cubic );

    peakfind::slope_state<peakfind::counter> state( w / 2 );
    
    double base_avg = dbase;
    size_t base_pos = 0, base_c = 0;

#if defined _DEBUG || defined DEBUG // debug
    std::ofstream o( "spectrum.txt" );
	SGFilter smoother( m );
#endif

    for ( size_t x = NH; x < nbrSamples - NH; ++x ) {
        double d1 = diff( &pY[x] );

        bool reduce = false;
        if ( d1 >= slope ) {
            if ( ( base_c = std::min( base_c, w * 2 ) ) )
                base_avg = std::accumulate( py.begin() + base_pos - base_c, py.begin() + base_pos, 0.0 ) / double(base_c);
            base_c = 0;
            reduce = state.process_slope( peakfind::counter( x, peakfind::Up ) );
        } else if ( d1 <= (-slope ) ) {
            base_c = 0;
            reduce = state.process_slope( peakfind::counter( x, peakfind::Down ) );
        } else {
            if ( state.stack_.size() < 2 ) {
                ++base_c;
                base_pos = x;
            }
        }

#if defined _DEBUG || defined DEBUG
        if ( 12 < pX[x] && pX[x] < 34 ) {
            o << std::fixed << std::setprecision(14) << x << "\t" << pX[x] << "\t" << pY[x] << "\t" << d1 << "\t" << smoother( &pY[x] )
              << "\t" << ( reduce ? "true" : "false");
            for ( int i = 0; i < int( state.stack_.size() ); ++i )
                o << "\tstack:" << state.stack_[i].type() << ", " << state.stack_[i].distance();
            o << std::endl;
        }
#endif
    
        if ( reduce ) {
            double baselevel = base_avg;
            std::pair< peakfind::counter, peakfind::counter > peak;
            while ( state.reduce( peak ) ) {
				if ( pX[ peak.second.tpos_ ] - pX[ peak.first.bpos_ ] >= peakwidth_ ) {
                    if ( !results_.empty() ) {
                        const peakinfo& prev = results_.back();
                        if ( peak.first.bpos_ - prev.first >= w )
                            baselevel = std::min( pY[peak.first.bpos_], pY[peak.second.tpos_] );
                    }
                    results_.push_back( peakinfo( peak.first.bpos_, peak.second.tpos_, baselevel ) );
                }
            }
        } 
        if ( atmz && atmz < pX[ x ] ) {
            atmz *= 4;
            diff = SGFilter( int( diff.coefficients().size() / 2 ) | 0x01, SGFilter::Derivative1, SGFilter::Cubic );
#if defined _DEBUG || defined DEBUG
			smoother = SGFilter( int( diff.coefficients().size() / 2 | 0x01 ) );
#endif
        }
    }

    if ( ! state.stack_.empty() ) {

        if ( state.stack_.top().type() == peakfind::Down )
            state.stack_.push( peakfind::counter( nbrSamples - 1, peakfind::None ) ); // dummy

        std::pair< peakfind::counter, peakfind::counter > peak;

        while ( state.reduce( peak ) ) {
            double baselevel = base_avg;
            if ( pX[ peak.second.tpos_ ] - pX[ peak.first.bpos_ ] >= peakwidth_ ) {
                if ( !results_.empty() ) {
                    const peakinfo& prev = results_.back();
                    if ( peak.first.bpos_ - prev.first >= w )
                        baselevel = std::min( pY[peak.first.bpos_], pY[peak.second.tpos_] );
                }
				results_.push_back( peakinfo( peak.first.bpos_, peak.second.tpos_, baselevel ) );
            }
        }
    }

    return results_.size();
}

bool
spectrum_processor::getFraction( areaFraction& frac, const double * pMasses, size_t size, double lMass, double hMass )
{
	frac = areaFraction(); // clear

    array_wrapper< const double > masses( pMasses, size );
    auto it = std::lower_bound( masses.begin(), masses.end(), lMass );
    if ( it == masses.end() )
        return false;
    frac.lPos = std::distance( masses.begin(), it );

    it = std::lower_bound( masses.begin(), masses.end(), hMass );
    if ( it == masses.end() )
        return false;
    frac.uPos = std::distance( masses.begin(), it );

    if ( frac.lPos < frac.uPos )
        --frac.uPos;

    frac.lFrac = ( masses[ frac.lPos ] - lMass ) / ( masses[ frac.lPos ] - masses[ frac.lPos - 1 ] );

    frac.uFrac = ( hMass - masses[ frac.uPos ] ) / ( masses[ frac.uPos + 1 ] - masses[ frac.uPos ] );

    if ( frac.uFrac < 0.0 )
        frac.uFrac = 0;

    return true;
}

double
spectrum_processor::area( const areaFraction& frac, double base, const double* pData, size_t nData )
{
    return areaCalculator<double>::area( frac, base, pData, nData );
}

double
spectrum_processor::area( const areaFraction& frac, double base, const int32_t* pData, size_t nData )
{
    return areaCalculator<int32_t>::area( frac, base, pData, nData );
}

double
spectrum_processor::area( const areaFraction& frac, double base, const int16_t* pData, size_t nData )
{
    return areaCalculator<int16_t>::area( frac, base, pData, nData );
}

double
spectrum_processor::area( const areaFraction& frac, double base, const int8_t* pData, size_t nData )
{
    return areaCalculator<int8_t>::area( frac, base, pData, nData );
}


