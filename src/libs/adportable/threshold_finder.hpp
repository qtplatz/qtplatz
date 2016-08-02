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

namespace adportable {

    struct threshold_finder {
    
        const bool findUp;
        const unsigned int nskip;
    
        threshold_finder( bool _findUp, unsigned int _nskip )
            : findUp( _findUp ), nskip( _nskip ) {
        }

        // find either raising index or falling index per peak
        template< typename const_iterator >
        void operator()( const_iterator&& begin, const_iterator&& end
                         , std::vector< uint32_t >& indecies
                         , typename std::iterator_traits< const_iterator >::value_type level ) {
            bool flag;
            auto it = begin;
            while ( it != end ) {
                if ( ( it = adportable::waveform_processor().find_threshold_element( it, end, level, flag ) ) != end ) {
                    if ( flag == findUp )                        
                        indecies.emplace_back( uint32_t( std::distance( begin, it ) ) );
                    adportable::advance( it, nskip, end );
                }
            }
        }

        // find both raising,falling indecies
        template< typename const_iterator >
        void operator()( const_iterator&& begin, const_iterator&& end
                         , std::vector< std::pair< uint32_t, uint32_t > >& indecies
                         , typename std::iterator_traits< const_iterator >::value_type level ) {
            bool flag;
            auto it = begin;
            while ( it != end ) {
                if ( ( it = adportable::waveform_processor().find_threshold_element( it, end, level, flag ) ) != end ) {
                    if ( flag == findUp ) {
                        indecies.emplace_back( uint32_t( std::distance( begin, it ) ), 0 ); // index, offset for up/down conter part
                    } else {
                        if ( !indecies.empty() )
                            indecies.back().second = uint32_t( std::distance( begin, it ) );
                    }
                }
            }
        }
        
    };

}
