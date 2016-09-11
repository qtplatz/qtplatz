// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2015 MS-Cheminformatics LLC
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

#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>
#include <numeric>

namespace adportable {

    struct average {
        template< typename Iterator >
        inline double operator()( Iterator it, size_t count ) {
            double sum = std::accumulate( it, it + count, 0 );
            return sum / count;
        }
    };

    struct stddev {
        template< typename Iterator >
        inline std::pair<double,double> operator()( Iterator it, size_t count ) {
            double mean = average()( it, count );
            double ssum = std::accumulate( it, it + count, 0.0, [&]( const double& sum, const auto& d ){
                    return sum + ( d - mean ) * ( d - mean );
                });
            return std::make_pair(sqrt( ssum / count ), mean );
        }
    };
    
}

