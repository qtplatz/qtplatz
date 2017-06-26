/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC
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

#include "histogram_processor.hpp"
#include "moment.hpp"
#include "debug.hpp"
#include "float.hpp"
#include <boost/format.hpp>
#include <cassert>
#include <cmath>
#include <cstring> // for memset()
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <stack>

using namespace adportable;


namespace adportable {
    namespace histogram {

        ///////////////////////////////
        struct distance {

            const double * pTimes_;
            const double * pCounts_;
            const double xInterval_;

            distance( const double * pTimes, const double * pCounts, const double& i ) : pTimes_( pTimes )
                                                                                       , pCounts_( pCounts )
                                                                                       , xInterval_( i ) {
            }
                                                                                         
            inline int operator()( size_t idx1, size_t idx2 ) const {
                return int( ( ( pTimes_[ idx2 ] - pTimes_[ idx1 ] ) / xInterval_ ) + 0.5 );
            }

            inline double slope( size_t idx1, size_t idx2 ) const {
                return ( pCounts_[ idx2 ] - pCounts_[ idx1 ] ) / ( pTimes_[ idx2 ] - pTimes_[ idx1 ] );
            }
        };

        ///////////////////////////////
        constexpr const size_t stack_size = 64;

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

            slope_state( size_t w = 5 ) : width_( w ) {}

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

            void clear() {
                while ( ! stack_.empty() )
                    stack_.pop();
            }

            bool process_slope( const T& t ) {
                if ( stack_.empty() ) {
                    stack_.push( t ); // Up|Down
                    return false;
                } else if ( stack_.top().type() == t.type() ) {
                    stack_.top()++;
                    return false;
                } else {
                    stack_.push( t );
                    return stack_.size() >= 3;
                }
            }
        };
    }
}

using namespace adportable;
using namespace adportable::histogram;

histogram_peakfinder::histogram_peakfinder( double xInterval, uint32_t width ) : xInterval_( xInterval )
                                                                               , width_( width < 3 ? 3 : width | 01 )
{
}
    
size_t
histogram_peakfinder::operator()( size_t nbrSamples, const double * pTimes, const double * pCounts )
{
    // input data series must be acquired from 'equal time distance' digitizer
    
    if ( pTimes == 0 || pCounts == 0 )
        return 0;

    // static const int width = 3;
    static const double slope = 0.1;

    slope_state< counter > state( width_ / 2 );
            
    for ( auto it = pCounts + 1; it < pCounts + nbrSamples - 1; ++it ) {

        bool reduce = false;

        size_t x = std::distance( pCounts, it );           // index of counts array
        int dt = int ( 0.5 + ( pTimes[ x ] - pTimes[ x - 1 ] ) / xInterval_ ); // sample distance to/from previous

        if ( dt >= 3 ) {
            state.process_slope( counter( x - 1, None ) );
            std::pair< counter, counter > peak;
            while ( state.reduce( peak ) )
                results_.emplace_back( peakinfo( peak.first.bpos_, peak.second.tpos_, 0, 0 ) );
            state.clear();
        }

        double d1 = ( -( it[ -1 ] ) + it[ 1 ] ) / 2;       // simple 1st delivertive

        int typ( 0 );

        if ( d1 >= slope ) {
            typ = 1;
            reduce = state.process_slope( counter( x, Up ) );
        } else if ( d1 <= (-slope) ) { // negative slope
            typ = 2;
            reduce = state.process_slope( counter( x, Down ) );
        } else if ( *it > 0 ) {
            typ = 3;
            state.stack_.top()++; // extend
        }
#if 0
        { // debug
            double pt = pTimes[ x - 1 ];
            if ( x < 50 )  {
                double t = pTimes[ x ];
                ADDEBUG() << boost::format( "data: %d\t%.4lf\t%d\t%d\td1=%g\treduce=%d\t%d " )
                    % x % ( t * 1e6 ) % dt % *it % d1 % reduce % state.stack_.size()
                          << ( typ == 1 ? "Up" : ( typ == 2 ? "Down" : "Hold" ) )
                          << ",\t" << ( state.stack_.empty() ? 0 : state.stack_.top().bpos_ );
            }
        }
#endif
        if ( reduce ) {
            std::pair< counter, counter > peak;
            while ( state.reduce( peak ) ) {
                uint32_t flag = uint32_t( pCounts[ peak.second.tpos_ ] );
                results_.emplace_back( peakinfo( peak.first.bpos_, peak.second.tpos_, 0, flag ) );
#if 0
                if ( x < 50 )
                    ADDEBUG() << boost::format( "\treduce: %d\t%d" ) % peak.first.bpos_ % peak.second.tpos_;
#endif
            }
        }

    }
    
    return results_.size();
}


////////////////////////////////////////////////

histogram_merger::histogram_merger( double xInterval, double threshold ) : xInterval_( xInterval )
                                                                         , threshold_( threshold )
{
}

size_t
histogram_merger::operator()( std::vector< peakinfo >& pkinfo, size_t nbrSamples
                              , const double * pMasses, const double * pTimes, const double * pCounts )
{
    if ( pkinfo.size() < 2 )
        return pkinfo.size();

    // copy (preserve original) version
    std::vector< peakinfo > results;

    for ( auto it = pkinfo.begin() + 1; it != pkinfo.end(); ++it ) {
        
        size_t x1 = (it - 1)->second;
        size_t x2 = it->first;

        int dt = int ( ( ( pTimes[ x2 ] - pTimes[ x1 ] ) / xInterval_ ) + 0.5 );

        if ( dt == 1 && pCounts[ x1 ] > 0.5 ) { // concatnate if true
            results.back().second = it->second; // merge previous
        } else {
            results.emplace_back( *it );
        }
    }

    pkinfo = results;

    return 0;
}
