// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include <limits>
#include <cmath>

namespace adportable {

    template<typename T> struct compare {

        static bool is_equal( T a, T b ) {
            return std::abs( a - b ) < std::numeric_limits<T>::epsilon();
        }

        static bool approximatelyEqual(T a, T b, T epsilon = std::numeric_limits<T>::epsilon() ) {
            return std::abs(a - b) <= ( (std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
        }

        static bool essentiallyEqual( T a, T b, T epsilon = std::numeric_limits<T>::epsilon() ) {
            return std::abs(a - b) <= ( (std::abs(a) > std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
        }

        static bool definitelyGreaterThan( T a, T b, T epsilon = std::numeric_limits<T>::epsilon() ) {
            return ( a - b ) > ( (std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
        }

        static bool definitelyLessThan( T a, T b, T epsilon = std::numeric_limits<T>::epsilon() ) {
            return ( b - a ) > ( (std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
        }
    };

}
