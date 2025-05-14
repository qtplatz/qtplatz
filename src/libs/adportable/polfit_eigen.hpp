// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC
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

#include "eigen.hpp"
#include <vector>
#include <cmath>
#include <optional>
#include <tuple>
#include <functional>

namespace adportable {

    template<typename T = double>
    class PolFit {
        static constexpr size_t _X = 0, _Y = 1;

        using MatrixT = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;
        using VectorT = Eigen::Matrix<T, Eigen::Dynamic, 1>;

    public:
        enum class Weighting {
            None,
            InvY,
            InvY2,
            InvY3
        };

        static
        std::optional< std::tuple< std::vector<T>, T > >
        fit( std::function< std::pair<T, T> ( int idx ) > xy_functor
            , size_t size
            , size_t degree
            , Weighting weighting = Weighting::None) {

            if (size < degree + 1)
                return {};

            const size_t n = size;
            MatrixT A(n, degree + 1);
            VectorT Y(n);
            VectorT W(n);

            for (size_t i = 0; i < n; ++i) {
                auto [x, y] = xy_functor(i);
                T w = weight(weighting, y);
                T x_pow = T(1);
                for (int j = 0; j <= degree; ++j) {
                    A(i, j) = x_pow * w;
                    x_pow *= x;
                }
                Y(i) = y * w;
                W(i) = w;
            }

            VectorT solution = A.colPivHouseholderQr().solve(Y);

            std::vector<T> coeffs(solution.data(), solution.data() + solution.size());

            T chisqr = T(0);
            for (size_t i = 0; i < n; ++i) {
                auto [x, y] = xy_functor(i);
                T y_est = estimate_y(coeffs, x);
                T resid = (y - y_est) * W(i);
                chisqr += resid * resid;
            }

            return std::make_tuple(coeffs, chisqr);
        }

        static T estimate_y(const std::vector<T>& coeffs, T x) {
            T y = T(0);
            T x_pow = T(1);
            for (auto c : coeffs) {
                y += c * x_pow;
                x_pow *= x;
            }
            return y;
        }

        static T standard_error( std::function< std::pair<T, T> ( int idx ) > xy_functor
                                 , size_t size
                                 // const std::vector<T>& x,
                                 // const std::vector<T>& y,
                                 , const std::vector<T>& coeffs) {

            // if (x.size() != y.size() || x.empty())
            //     return T(0);
            if ( size == 0 )
                return T{0};

            T sse = T{0};
            for (size_t i = 0; i < size; ++i) {
                auto [x,y] = xy_functor(i);
                T diff = estimate_y(coeffs, x - y);
                sse += diff * diff;
            }
            return std::sqrt( sse / size );
        }

    private:
        static T weight(Weighting mode, T y) {
            using std::abs;
            switch (mode) {
                case Weighting::None: return T(1);
                case Weighting::InvY: return abs(y) > T(1e-12) ? T(1) / abs(y) : T(1);
                case Weighting::InvY2: return y * y > T(1e-12) ? T(1) / (y * y) : T(1);
                case Weighting::InvY3: return abs(y * y * y) > T(1e-12) ? T(1) / abs(y * y * y) : T(1);
            }
            return T(1);
        }
    };

}
