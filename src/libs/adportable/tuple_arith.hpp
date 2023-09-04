// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC
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

#pragma once

#include <utility>

namespace adportable {
    namespace tuple_arith {

        template< class Tuple, typename Scalar, std::size_t... Is >
        Tuple tuple_multiplier( const Tuple& t, Scalar s, std::index_sequence<Is...> ){
            Tuple a;
            ((std::get< Is >(a) = std::get<Is>(t) * s), ...);
            return a;
        }

        template< class Tuple, typename Scalar, std::size_t... Is >
        Tuple tuple_divider( const Tuple& t, Scalar s, std::index_sequence<Is...> ){
            Tuple a;
            ((std::get< Is >(a) = std::get<Is>(t) / s), ...);
            return a;
        }

        template< typename... Args, typename Scalar >
        std::tuple<Args... > operator*(const std::tuple<Args...>& a, Scalar s ) {
            return tuple_multiplier( a, s, std::index_sequence_for< Args... >{} );
        }

        template< typename... Args, typename Scalar >
        std::tuple<Args... > operator/(const std::tuple<Args...>& a, Scalar s ) {
            return tuple_divider( a, s, std::index_sequence_for< Args... >{} );
        }
    }
}
