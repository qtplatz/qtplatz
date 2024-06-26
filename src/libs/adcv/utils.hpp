/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
**************************************************************************/

#pragma once

#include "advision_global.hpp"
#include <boost/numeric/ublas/matrix.hpp>

namespace advision {

    template< typename T > struct max_value {
        inline std::pair< T, T > operator()( const boost::numeric::ublas::matrix< T >& m ) const {
            T maxValue = { m(0, 0) };
            for ( int i = 0; i < m.size1(); ++i ) {
                for ( int j = 0; j < m.size2(); ++j )
                    maxValue = std::max( mm.first, m( i, j ) );
            }
            return maxValue;
        }
    };
}
