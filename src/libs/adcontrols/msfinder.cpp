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

#include "msfinder.hpp"
#include "massspectrum.hpp"
#include <algorithm>
#include <iterator>

using namespace adcontrols;

namespace adcontrols { 
    namespace detail {

        template< typename It >
        struct orderd_mass_finder {
            It beg_;
            It end_;
            double tolerance_;
            std::pair< It, It > range_;

            orderd_mass_finder( It beg, It end, double tolerance )
                : beg_( beg ), end_( end ), tolerance_( tolerance ), range_( 0, 0 ) {
            }

            std::pair< size_t, size_t > range() const {
                return std::make_pair( std::distance( beg_, range_.first ), std::distance( beg_, range_.second ) );
            }

            inline bool unique() const { return std::distance( range_.first, range_.second ) <= 1;  }
            inline size_t first() const { return std::distance( beg_, range_.first ); }
            inline size_t second() const { return std::distance( beg_, range_.second ); } // CAUTION ! -- second may be points 'end' of the array which does not actually exist
                    
            size_t closest( double mass ) {

                if ( range_.first && range_.second ) {
                    auto it = std::min_element( range_.first, range_.second, [&] ( decltype(*range_.first)& a, decltype(*range_.first)& b ){ return std::abs( a - mass ) < std::abs( b - mass ); } );
                    return std::distance( it, beg_ );
                }

                return size_t( -1 );
            };

            bool operator()( double target_mass ) {
                
                if ( target_mass < *beg_ || *(end_ - 1) < target_mass )
                    return false;
                
                range_.first = std::lower_bound( beg_, end_, target_mass - tolerance_ );
                if ( range_.first != end_ ) {
                    
                    double d = std::abs( *range_.first - target_mass );
                    if ( d > tolerance_ )
                        return false;
                    
                    range_.second = std::lower_bound( range_.first, end_, target_mass + tolerance_ ); // next to the last effective point

                    return true;

                }
                return false;
            }
        };
    }
}

MSFinder::~MSFinder()
{
}

MSFinder::MSFinder( const MSFinder& t ) : width_( t.width_ )
                                        , toleranceMethod_( t.toleranceMethod_ )
                                        , findAlgorithm_( t.findAlgorithm_ )
{
}

MSFinder::MSFinder( double width, idFindAlgorithm a, idToleranceMethod w ) : width_(width)
                                                                       , toleranceMethod_( w )
                                                                       , findAlgorithm_( a )
{
}

size_t
MSFinder::operator()( const MassSpectrum& ms, double mass )
{
    double tolerance = (toleranceMethod_ == idToleranceDaltons) ? width_ : (mass * width_ / 1.0e6);
    
    detail::orderd_mass_finder<const double *> finder( ms.getMassArray(), ms.getMassArray() + ms.size(), tolerance );

    if ( finder( mass ) ) {
        
        if ( finder.unique() )
            return finder.first();
    
        if ( findAlgorithm_ == idFindLargest ) {

            const double * intens = ms.getIntensityArray();

            std::pair< size_t, size_t > range = finder.range();
            auto it = std::max_element( intens + range.first, intens + range.second );

            return std::distance( intens, it );

        } else if ( findAlgorithm_ == idFindClosest ) {

            return finder.closest( mass );
            
        }

    }
    return npos;
}


