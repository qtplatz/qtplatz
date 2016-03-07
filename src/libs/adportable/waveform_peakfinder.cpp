/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "waveform_peakfinder.hpp"
#include "spectrum_processor.hpp" // tic
#include "polfit.hpp"
#include <boost/variant.hpp>

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

    struct waveform_peakfinder_i {

        std::function< double( size_t idx, int& iw )>& fpeakw_;
        const double rms_;
        const double dbase_;

        inline void internal_update( size_t idx, double& peakw, int& iw, int& m, int& NH, double& slope ) const {
            peakw = fpeakw_( idx, iw );
            m = ( iw < 5 ) ? 5 : iw | 0x01;    // n-order for SGFilter
            NH = m / 2;
            slope = rms_ / double( iw * 16 );
        }

        waveform_peakfinder_i( std::function< double( size_t idx, int& n )>& fpeakw, double dbase, double rms )
            : fpeakw_( fpeakw )
            , rms_( rms )
            , dbase_( dbase ) {
        }
        
        template< typename _fx, typename Ty >
        size_t find ( _fx fx, const Ty * pY, size_t beg, size_t end
                      , std::vector< adportable::waveform_peakfinder::peakinfo >& results  ) const {
            
            if ( pY == 0 || ( end - beg ) < 7 )
                return 0;

            int iw = 5;
            int m = iw;
            int NH = m / 2;
            double peakw = fpeakw_( beg, iw );
            double slope = double( rms_ ) / double( iw * 16 );
            internal_update( beg, peakw, iw, m, NH, slope );
            
            SGFilter diff( m, SGFilter::Derivative1, SGFilter::Cubic );

            peakfind::slope_state<peakfind::counter> state( iw / 2 );
    
            size_t base_pos = 0, base_c = 0;

            for ( size_t x = beg + NH; x < end - NH; ++x ) {

                double d1 = diff( &pY[x] );

                bool reduce = false;
                if ( d1 >= slope ) {
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

                if ( reduce ) {
                    std::pair< peakfind::counter, peakfind::counter > peak;
                    while ( state.reduce( peak ) ) {
                        if ( fx( peak.second.tpos_ ) - fx( peak.first.bpos_ ) >= peakw ) {
                            results.push_back( waveform_peakfinder::peakinfo( peak.first.bpos_, peak.second.tpos_, 0 ) );
                        }
                    }
                    internal_update( x, peakw, iw, m, NH, slope );
                } 
            }

            if ( ! state.stack_.empty() ) {

                if ( state.stack_.top().type() == peakfind::Down )
                    state.stack_.push( peakfind::counter( end - 1, peakfind::None ) ); // dummy

                std::pair< peakfind::counter, peakfind::counter > peak;

                while ( state.reduce( peak ) ) {
                    if ( fx( peak.second.tpos_ ) - fx( peak.first.bpos_ ) >= peakw ) {
                        results.push_back( waveform_peakfinder::peakinfo( peak.first.bpos_, peak.second.tpos_, 0 ) );
                    }
                }
            }
            
            return results.size();
        }
    };
}

///////////////
waveform_peakfinder::waveform_peakfinder( std::function< double( size_t idx, int& n )> fpeakw )
    : fpeakw_( fpeakw )
    , dbase_( 0 )
    , rms_( 0 )
{
}

/** \brief find peaks from waveform, intended input is the time-of-flight mass spectrum
 * 
 * Internally, it compute rms and baseline level by using TIC calculation algorithm.
 * Computed baseline level can be returned by dbase() method, and rms() method for RMS.
 * dbase is used for peak height determination, and rms is used for slope determination.
 */
template<> size_t
waveform_peakfinder::operator()( std::function< double ( size_t ) > fx
                                 , const double * pY
                                 , size_t beg, size_t end
                                 , std::vector< waveform_peakfinder::peakinfo >& results )
{
    // compute baseline level and rms
    spectrum_processor::tic( end - beg, &pY[beg], dbase_, rms_ );

    // find peaks
    waveform_peakfinder_i finder( fpeakw_, dbase_, rms_ );

    if ( finder.find( fx, pY, beg, end, results ) ) {
        adportable::Moment< decltype(fx) > moment( fx );
        for ( auto& pk: results ) {
            auto it = std::max_element( pY + pk.spos, pY + pk.epos );
            pk.tpos = std::distance( pY, it );
            pk.height = *it - dbase_;
            double threshold = pk.height / 2 + dbase_;
            pk.centreX = moment.centreX( pY, threshold, uint32_t(pk.spos), uint32_t(pk.tpos), uint32_t(pk.epos) );
            pk.xleft = moment.xLeft();
            pk.xright = moment.xRight();
        }
        return results.size();        
    }
    return 0;
}

bool
waveform_peakfinder::fit( std::function< double ( size_t ) > fx
                         , const double * pY
                         , size_t spos
                         , size_t tpos
                         , size_t epos
                         , waveform_peakfinder::parabola& pk )
{
    pk.spos = spos;
    pk.tpos = tpos;
    pk.epos = epos;
    if ( ( epos - spos ) + 1 >= 5 ) {
        std::vector< double > x, y, r;
        for ( size_t i = pk.spos; i <= pk.epos; ++i ) {
            x.push_back( fx( i ) );
            y.push_back( pY[ i ] );
        }
        if ( adportable::polfit::fit( x.data(), y.data(), x.size(), 3, r ) ) {
            pk.a = r[0];  // y = a + bx + cx^2
            pk.b = r[1];
            pk.c = r[2];
            pk.centreX = (-pk.b) / ( 2 * pk.c ); // first delivative of quadratic eq, at x = 0;
            pk.height = ( pk.a + pk.b * pk.centreX + pk.c * ( pk.centreX * pk.centreX ) ) - dbase_;
        }
        return true;
    }
    return false;
}

double
waveform_peakfinder::dbase() const
{
    return dbase_;
}

double
waveform_peakfinder::rms() const
{
    return rms_;
}
