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

namespace adcontrols {

    namespace mschromatogramextractor {

        template< typename It > struct accumulate {
            size_t size_;
            const It xbeg_;
            const It xend_;        
            const It y_;

            accumulate( It x, It y, size_t size ) : size_( size )
                                                  , xbeg_( x )
                                                  , xend_( x + size )
                                                  , y_( y ) {
            }
        
            double operator()( double lMass, double uMass ) const {
                if ( size_ ) {
                    auto lit = std::lower_bound( xbeg_, xend_, lMass );
                    if ( lit != xend_ ) {
                        auto bpos = std::distance( xbeg_, lit );
                        auto uit = std::lower_bound( xbeg_, xend_, uMass );
                        if ( uit == xend_ )
                            uit--;
                        while ( uMass < *uit )
                            --uit;
                        auto epos = std::distance( xbeg_, uit );
                        if ( bpos > epos )
                            epos = bpos;

                        return std::accumulate( y_ + bpos, y_ + epos, 0.0 );
                    }
                }
                return 0.0;
            }
        };
    }
}

