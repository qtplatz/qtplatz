// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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
/*
 * FFT and related algolism for spectral peak picking
 * Originally created by T. Hondo, '90, 01/09
 */

#pragma once

namespace adportable {

    namespace digital_filter {

        struct make_power2 {
            static size_t size2( size_t n ) {
                size_t N = 2;
                while ( N < n )
                    N <<= 1;
                return N;
            }

            template< typename InputIt, typename OutputIt, typename UnaryOperation >
            static void transform( InputIt first, InputIt last, OutputIt d_first, UnaryOperation op ) {
                auto it = std::transform( first, last, d_first, op );
                std::fill( d_first + std::distance( first, last ), it, op( *(last - 1) ) );
            }
        };

        struct cutoff_index {
            size_t operator()( double interval /*second*/, size_t N, double freq /* Hz */ ) {
                return N * interval * freq;
            }
            std::pair<size_t, size_t> operator()( double interval /*second*/, size_t N, std::pair<double,double> freq /* Hz */ ) {
                return { N * interval * freq.first, N * interval * freq.second };
            }
        };

        struct filter {
            template< typename T > void low_pass( std::vector< T >& a, const size_t index ) const { // high cut
                if ( index && a.size() > index * 2 )
                    std::fill( a.begin() + index, a.begin() + (a.size() - index - 1), 0 );
            }

            template< typename T > void high_pass( std::vector< T >& a, const size_t index ) const { // low cut
                if ( index ) {
                    auto llimit = index / 2;
                    for ( size_t i = 0; i < llimit; ++i ) {  // triangle filter
                        double factor = double(i) / llimit;
                        a[ i + 1 ] *= factor; // save DC
                        a[ a.size() - i - 1 ] *= i / llimit;
                    }
                }
            }
        };

    }

}
