/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

namespace acqrscontrols {

    struct threshold_action_finder {

        template< typename threshold_result_ptr >
        bool operator()( threshold_result_ptr result, std::shared_ptr< const adcontrols::threshold_action > action ) const {

            // Action for detected slope
            if ( !result->indecies().empty() && action->enable ) {

                const auto& meta = result->data()->meta_;

                if ( action->enableTimeRange ) {
            
                    uint32_t beg = uint32_t( ( action->delay - meta.initialXOffset ) / meta.xIncrement );
                    uint32_t end = uint32_t( ( action->delay + action->width - meta.initialXOffset ) / meta.xIncrement );

                    auto it = std::lower_bound( result->indecies().begin(), result->indecies().end(), beg );
                    if ( it != result->indecies().end() ) {
                        if ( *it < end ) {
                            result->setFoundAction( *it, std::make_pair( beg, end ) );
                            return true;
                        }
                    }
                    return false;

                } else {

                    // no time range specified
                    result->setFoundAction( *result->indecies().begin(), std::make_pair( 0, uint32_t( result->indecies().size() - 1 ) ) );

                    return true;

                }
            }
            return false;
            
        }
        
    };
}

