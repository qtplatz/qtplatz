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

using namespace adportable;
using namespace adportable::histogram;

histogram_peakfinder::histogram_peakfinder( double xInterval ) : xInterval_( xInterval )
{
}

    
size_t
histogram_peakfinder::operator()( size_t nbrSamples, const double * pTimes, const double * pCounts )
{
    // input data series must be acquired from 'equal time distance' digitizer
    
    if ( pTimes == 0 || pCounts == 0 )
        return 0;

    std::vector< std::pair< size_t, size_t > > clusters;
    
    distance distance( pTimes, pCounts, xInterval_ );

    // find contenous data reagion

    size_t idx = 0;
    while ( idx < nbrSamples - 1 ) {
        
        if ( distance( idx, idx + 1 ) == 1 ) {
            
            size_t first = idx;
            
            while ( ( distance( idx, idx + 1 ) == 1 ) && ( idx < nbrSamples - 1 ) )
                ++idx;

            auto last = idx;

            if ( ( last - first + 1 ) >= 3 )
                clusters.emplace_back( first, idx );
            
        }

        ++idx;
    }
    
    static const int width = 3;
    static const double slope = 0.0;
    
    for ( auto& cluster: clusters ) {
        if ( ( cluster.second - cluster.first ) > width ) {

            //ADDEBUG() << "cluster: " << cluster.first << ", " << cluster.second;

            slope_state< counter > state( width / 2 );

            bool reduce = false;
            for ( auto it = pCounts + cluster.first + 1; it < pCounts + cluster.second; ++it ) {
                double d1 = ( -( it[ -1 ] ) + it[ 1 ] ) / 2;
                size_t x = std::distance( pCounts, it );
                if ( d1 >= slope )
                    reduce = state.process_slope( counter( x, Up ) );
                else if ( d1 <= (-slope) )
                    reduce = state.process_slope( counter( x, Down ) );
                if ( reduce ) {
                    std::pair< counter, counter > peak;
                    while ( state.reduce( peak ) ) {
                        // ADDEBUG() << peak.first.bpos_ << ", " << peak.second.tpos_;
                        results_.emplace_back( peakinfo( peak.first.bpos_, peak.second.tpos_, 0 ) );
                    }
                }
            }
        }
    }
    
    return results_.size();
}



