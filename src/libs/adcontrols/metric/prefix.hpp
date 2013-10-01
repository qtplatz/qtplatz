/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef PREFIX_HPP
#define PREFIX_HPP

#pragma once

#include <cmath>

namespace adcontrols {  namespace metric {

        enum prefix {
            yacto = -24
            , zepto = -21
            , atto  = -18
            , femto = -15
            , pico  = -12
            , nano  = -9
            , micro = -6
            , milli = -3
            , centi = -2
            , deci  = -1
            , base  = 0  // SI base units
            , deca  = 1
            , hecto = 2
            , kilo  = 3
            , mega  = 6
            , giga  = 9
            , tera  = 12
            , peta  = 15
            , exa   = 18
            , zetta = 21
            , yotta = 24
        };

        template<typename T> inline T scale_to( prefix to, const T t,prefix from = base ) {
            return T( t * std::pow( 10.0, from - to ) );
        }
        template<typename T, prefix to> inline T scale_to( T t, prefix from = base ) {
            return T( t * std::pow( 10.0, from - to ) );            
        }

        // helper functions
        template<typename T> inline T scale_to_base( T t, prefix from ) {
            return scale_to<T, base>( t, from );
        }

        template<typename T> inline T scale_to_pico( T t, prefix from = base ) {
            return scale_to<T, pico>( t, from );
        }
        template<typename T> inline T scale_to_nano( T t, prefix from = base ) {
            return scale_to<T, nano>( t, from );
        }
        template<typename T> inline T scale_to_micro( T t, prefix from = base ) {
            return scale_to<T, micro>( t, from );
        }
        template<typename T> inline T scale_to_milli( T t, prefix from = base ) {
            return scale_to<T, milli>( t, from );
        }

    } // metric

}

#endif // PREFIX_HPP
