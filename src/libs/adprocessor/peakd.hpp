/**************************************************************************
** Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2024 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <Eigen/Dense>
#include <utility>

namespace adprocessor {

    template< typename T, size_t Ndim, size_t Ncomp >
    class PeakDecomposition {
        Eigen::Matrix< T, Ndim, Ncomp > A_;

        template< class Tuple, std::size_t... Is>
        void decomposition_impl( Eigen::Vector< T, Ndim >& b, const Tuple& t, std::index_sequence<Is...>) {
            ((b(Is) = std::get<Is>(t)), ...);
        }
    public:
        PeakDecomposition() : A_( Eigen::Matrix< T, Ndim, Ncomp >::Zero() ) {}
        PeakDecomposition( const Eigen::Matrix< T, Ndim, Ncomp >& a ) : A_( a ) {}

        template<typename... Args>
        std::pair< Eigen::Vector< T, Ndim >, Eigen::Vector< T, Ncomp > > operator()( Args... args ) {
            Eigen::Vector< T, Ndim > b;
            decomposition_impl( b, std::make_tuple( args... ), std::index_sequence_for< Args...>{} );
            return std::make_pair( b, A_.colPivHouseholderQr().solve(b) );
        }
    };

}
