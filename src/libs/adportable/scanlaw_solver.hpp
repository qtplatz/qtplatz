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
#include <numeric>
#include "debug.hpp"

namespace adportable {

    class scanlaw_solver {
        double a_;
        double b_;
        static constexpr const std::pair< double, double > zero = {0.0, 0.0};
        std::vector< std::pair< double, double > > d_; // sqrt(m), t
    public:
        scanlaw_solver() : a_(0), b_(0) {}

        scanlaw_solver& operator << ( std::pair< double, double >&& mt ) {
            d_.emplace_back( std::get<1>(mt), std::sqrt( std::get<0>(mt) ) );  // time,sqrt(mass)
            return *this;
        }
        // histrical
        scanlaw_solver( const std::pair< double, double >& m
                        , const std::pair< double, double >& t ) {
            d_.emplace_back( std::get<0>(t), std::sqrt( std::get< 0 >(m) ) );  // time,sqrt(mass)
            d_.emplace_back( std::get<1>(t), std::sqrt( std::get< 1 >(m) ) );  // time,sqrt(mass)
            fit();
        }

        void fit() {
            auto [x,y]
                = std::accumulate(d_.begin(), d_.end(), zero, [](const auto& a, const auto& b){
                    return std::make_pair(a.first + b.first, a.second + b.second); });
            auto [xx,xy]
                = std::accumulate(d_.begin(), d_.end(), zero, [](const auto& a, const auto& b){
                    return std::make_pair( a.first + b.first * b.first, a.second + b.first * b.second ); });

            a_ = ( d_.size() * xy - x*y ) / ( d_.size() * xx - x*x );
            b_ = ( xx*y - xy*x ) / (d_.size() * xx - x*x);
        }

        // compute mass from time
        inline double mass( double t ) const {
            return ( a_ * t + b_ ) * ( a_ * t + b_ );
        }

        // compute time from mass
        inline double time( double m ) const {
            return (std::sqrt(m) - b_) / a_;
        }

        inline double delta_t( double dm, double m ) const {
            return time( m + dm ) - time( m );
        }
        inline double a() const { return a_; }
        inline double b() const { return b_; }
        inline const std::vector< std::pair< double,double> >& d() const { return d_; }
    };
}
