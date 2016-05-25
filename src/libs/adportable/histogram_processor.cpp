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

#include <cmath>
#include <cstring> // for memset()
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <stack>

using namespace adportable;


namespace adportable {
    namespace histogram {
        
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

    for ( auto& cluster: clusters ) {

        auto it = std::max_element( pCounts + cluster.first, pCounts + cluster.second + 1 );
        auto idx = std::distance( pCounts, it );

        if ( ( distance.slope( cluster.first, idx ) > 0 ) &&
             ( distance.slope( idx, cluster.second ) < 0 ) ) {

            results_.emplace_back( peakinfo( cluster.first, cluster.second, 0 ) );

        }
        
    }
    
    return results_.size();
}



