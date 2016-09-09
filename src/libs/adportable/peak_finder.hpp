/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include "waveform_processor.hpp"
#include "advance.hpp"
#include <iterator>
//
#include <iostream>

namespace adportable {

    // single ion peak lookup
    
    struct peak_finder {
    
        const int sign;
    
        peak_finder( bool up ) : sign( up ? 1 : -1 ) {
        }

        template< typename const_iterator, typename index_type >
        void operator()( const_iterator&& begin
                         , const_iterator&& end
                         , std::vector< index_type >& indecies
                         , typename std::iterator_traits< const_iterator >::value_type level
                         , size_t offset = 0 ) {

            auto slope = std::abs( level );
            auto it = begin;
            ++it;
            while ( it < ( end - 1 )) {
                auto d = ( -( it[ -1 ] ) + it[ 1 ] );                 
                bool found = ( sign < 0 ) ? ( d < -slope ) : ( d > slope );
                if ( found ) {
                    uint32_t begIdx = std::distance( begin, it - 1 );
                    uint32_t apex( 0 ), endIdx(0);
                    int32_t value( 0 );

                    if ( sign < 0 ) {    // negative peak

                        while ( ++it < ( end - 1 ) ) {
                            if ( it[ -1 ] > it[ 0 ] && it[ 0 ] < it[ 1 ] ) {
                                value = *it;
                                apex = std::distance( begin, it );
                                break;
                            }
                        }
                        while ( it < ( end - 1 ) && *it < *(it + 1) )
                            ++it;

                    } else {             // positive peak
                        
                        while ( ++it < ( end - 1 ) ) {
                            if ( it[ -1 ] < it[ 0 ] && it[ 0 ] > it[ 1 ] ) {
                                value = *it;
                                apex = std::distance( begin, it );
                                break;
                            }                            
                        }
                        while ( it < ( end - 1 ) && *it < *(it + 1) )
                            ++it;                        
                    }
                    if ( apex )
                        indecies.emplace_back( begIdx, std::distance(begin, it), apex, value, *it ); // first,second,apex,value,base
                }
                ++it;
            }
        }
        
    };

}
