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

using namespace adcontrols;

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
    const double * begin = ms.getMassArray();
    const double * end = begin + ms.size();
    double lMass = (toleranceMethod_ == idToleranceDaltons) ? (mass - width_) : (mass - (mass * width_ / 1.0e6));
    double hMass = (toleranceMethod_ == idToleranceDaltons) ? (mass + width_) : (mass + (mass * width_ / 1.0e6));

    if ( findAlgorithm_ == idFindLargest ) {

        auto lIt = std::lower_bound( begin, end, lMass );
        if ( lIt != end && std::abs(*lIt - mass ) < width_) {
            auto hIt = std::lower_bound( lIt, end, hMass );

            auto it = std::max_element( lIt, hIt );
            return std::distance( begin, it );

        }
        return npos;

    }
    else if ( findAlgorithm_ == idFindClosest ) {

        auto it = std::lower_bound( begin, end, mass );
        if ( lMass <= *it && *it <= hMass ) {
            if ( it != begin ) {
                if ( std::abs( *it - mass ) > std::abs( *(it - 1) - mass ) )
                    --it;
                return std::distance( begin, it );
            }
        }
    }

    return npos;
}


