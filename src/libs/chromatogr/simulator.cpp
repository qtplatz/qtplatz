/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "simulator.hpp"
#include <numeric>
#include <boost/math/distributions/normal.hpp>

using namespace chromatogr::simulator;

peak::peak( double retention_time
            , double theoretical_plate
            , double height ) : retention_time_( retention_time )
                              , theoretical_plate_( theoretical_plate )
                              , height_( height )
                              , sigma_(0)
                              , distribution_height_( 0 )
{
    if ( theoretical_plate_ > 0 ) {
        sigma_ = retention_time_ / std::sqrt( theoretical_plate_ );
        boost::math::normal_distribution< double > nd ( retention_time_, sigma_ );
        distribution_height_ = boost::math::pdf( nd, retention_time_ );
    }
}

peak&
peak::operator = ( const peak& pk )
{
    retention_time_ = pk.retention_time_;
    theoretical_plate_ = pk.theoretical_plate_;
    height_ = pk.height_;
    sigma_ = pk.sigma_;
    distribution_height_ = pk.distribution_height_;
    return *this;
}

/**
 *   N = (tR / sigma)^2
 */
double
peak::intensity( double t ) const
{
    if ( sigma_ > 0 &&
         ( retention_time_ - ( sigma_ * 4 ) ) < t && t < ( retention_time_ + ( sigma_ * 4 ) ) ) {
        
        boost::math::normal_distribution< double > nd ( retention_time_, sigma_ );
        return ( boost::math::pdf( nd, t ) / distribution_height_ ) * height_;

    }
    return 0;
}

//////////////
void
peaks::operator << ( const peak& pk )
{
    peaks_.push_back( pk );
}

double
peaks::intensity( double t ) const
{
    return std::accumulate( peaks_.begin(), peaks_.end(), double(0), [t] ( double a, const peak& pk ) { return a + pk.intensity( t ); } );
}
