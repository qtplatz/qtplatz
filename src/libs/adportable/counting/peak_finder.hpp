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

    namespace counting {

        // single ion peak lookup

        // find peak top
        template< bool positive > struct find_peak {
            template< typename const_iterator >
            const_iterator operator()( const_iterator& it, const_iterator& end ) const;
        };

        template< bool findPositive >
        struct peak_finder {

            template< typename const_iterator >        
            inline auto /* typename const_iterator::value_type */ diferential3( const_iterator it ) const {
                return ( -( it[ -1 ] ) + it[ 1 ] ) / 2;
            }
                
            peak_finder() {
            }

            template< typename const_iterator, typename index_type >
            void operator()( const_iterator&& begin
                             , const_iterator&& end
                             , std::vector< index_type >& indices
                             , typename std::iterator_traits< const_iterator >::value_type level
                             , size_t offset = 0 ) {

                auto slope = std::abs( level );
                auto it = begin + offset + 1;
                while ( it < ( end - 1 )) {
                    auto d = diferential3( it );
                    bool found = ( findPositive ) ? ( d > slope ) : ( d < -slope );
                    if ( found ) {
                        const_iterator it0( it - 1 ), apex( end );
                        
                        apex = find_peak< findPositive >()( it, end );

                        if ( apex != end )
                            indices.emplace_back( std::distance( begin, it0 )
                                                   , std::distance( begin, it )
                                                   , std::distance( begin, apex )
                                                   , *apex, *it ); // first,second,apex,value,base(last value)
                    }
                    ++it;
                }
            }
        
        };

        // positive peak
        template<>
        template< typename const_iterator >
        inline const_iterator find_peak<true>::operator()( const_iterator& it, const_iterator& end ) const
        {
            const_iterator apex( end );
            while ( ++it < ( end - 1 ) ) {
                if ( it[ -1 ] < it[ 0 ] && it[ 0 ] > it[ 1 ] ) {
                    apex = it;
                    while ( it < ( end - 1 ) && *it > *(it + 1) )
                        ++it;
                    return apex;
                }                            
            }
            return end;
        }

        // negative
        template<>
        template< typename const_iterator >
        inline const_iterator find_peak<false>::operator()( const_iterator& it, const_iterator& end ) const
        {
            const_iterator apex( end );
            while ( ++it < ( end - 1 ) ) {
                if ( it[ -1 ] > it[ 0 ] && it[ 0 ] < it[ 1 ] ) {
                    apex = it;
                    while ( it < ( end - 1 ) && *it < *(it + 1) )
                        ++it;
                    return apex;
                }
            }
            return end;
        }


    } // counting
} // adportable
