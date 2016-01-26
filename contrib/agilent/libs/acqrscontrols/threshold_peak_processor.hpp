/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include <adcontrols/threshold_action.hpp>
#include <limits>
#include <cmath>

namespace acqrscontrols {

    // threshold: 0,0,0,0,1,1,0,0,0
    // indecies:  4,5

    struct threshold_peak_processor {
        
        template< typename threshold_result_ptr >
        size_t accumulate( threshold_result_ptr result, double tof, double window ) const {

            if ( !result->indecies().empty() ) {
                
                // tof equals zero
				if ( std::abs( tof ) <= std::numeric_limits<double>::epsilon() )
                    return result->indecies().size(); // return total ion count

                const auto& meta = result->data()->meta_;
                
                uint32_t beg = uint32_t( ( ( tof - window / 2 ) - meta.initialXOffset ) / meta.xIncrement );
                uint32_t end = uint32_t( ( ( tof + window / 2 ) - meta.initialXOffset ) / meta.xIncrement );
                
                auto lower = std::lower_bound( result->indecies().begin(), result->indecies().end(), beg );

                if ( lower != result->indecies().end() ) {

                    auto upper = std::upper_bound( result->indecies().begin(), result->indecies().end(), end );
                    return std::distance( lower, upper );
                }

            }
            return 0;
        }
        
    };
}

