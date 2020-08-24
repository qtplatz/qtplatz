/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <cmath>

namespace adportable {

    struct scanlaw_solver {
        double a_;
        double b_;
        scanlaw_solver( const std::pair< double, double >& m
                        , const std::pair< double, double >& t ) {
            b_ = (std::sqrt(m.second) - std::sqrt(m.first))/(t.second - t.first);
            a_ = std::sqrt(m.first) - ( b_ * 10e-6 );
        }
        inline double mass( double t ) const {
            return ( a_ + ( b_ * t ) ) * ( a_ + ( b_ * t ) );
        }
        inline double time( double m ) const {
            return (-a_ + std::sqrt(m)) / b_;
        }
        inline double delta_t( double dm, double m ) const {
            return time( m + dm ) - time( m );
        }
        inline double a() const { return a_; }
        inline double b() const { return b_; }
    };
}
