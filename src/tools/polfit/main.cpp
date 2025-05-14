/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <boost/program_options.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <adportable/csv_reader.hpp>
#include <adportable/polfit_eigen.hpp>
#include <Eigen/Dense>

struct visitor : boost::static_visitor< double > {
    double operator()( int x ) const {
        return double(x);
    }
    double operator()( double x ) const {
        return x;
    }
    double operator()( const std::string& x ) const {
        return std::strtod(x.c_str(), nullptr);
    }
    double operator()( boost::spirit::x3::unused_type ) const {
        return 0;
    }
};

template<typename T = double>
std::optional<std::tuple<T, T, T>> fit_normal_distribution(
    std::function<std::pair<T, T>(size_t)> xy_functor,
    size_t size,
    typename adportable::PolFit<T>::Weighting weighting = adportable::PolFit<T>::Weighting::None ) {
    using PolFitT = adportable::PolFit<T>;

    auto log_functor = [&](int idx) -> std::pair<T, T> {
        auto [x, y] = xy_functor(idx);
        if (y > T(0))
            return { x, std::log(y) };
        else
            return { x, std::log(std::numeric_limits<T>::min()) }; // Avoid -inf
    };

    auto polyfit_result = PolFitT::fit(log_functor, size, 2, weighting);
    if (!polyfit_result)
        return {};

    auto [coeffs, chisqr] = *polyfit_result;
    T a0 = coeffs[0];
    T a1 = coeffs[1];
    T a2 = coeffs[2];

    if (a2 >= T(0))
        return {}; // Not a valid Gaussian if a2 >= 0

    T sigma = std::sqrt(-1 / (2 * a2));
    T mu = a1 * sigma * sigma;
    T A = std::exp(a0 + (mu * mu) / (2 * sigma * sigma));

    return std::make_tuple(mu, sigma, A); // mean, stddev, amplitude
}


namespace po = boost::program_options;

int
main(int argc, char *argv[])
{
    po::variables_map vm;
    po::options_description description( argv[0] );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "input-files",  po::value< std::vector< std::string > >(),  "input files" )
            ( "x",  po::value< size_t > ()->default_value(0),  "colunn-x" )
            ( "y",  po::value< size_t > ()->default_value(1),  "colunn-y" )
            ( "degree", po::value< size_t > ()->default_value(1),  "polynomial degree" )
            ( "fit", po::value< std::string > (),  "fit=nd normal distribution fitting" )
            ( "skip",  po::value< size_t > (), "skip first n-lines" )
            ;
        po::positional_options_description p;
        p.add( "input-files",  -1 );
        po::store( po::command_line_parser( argc, argv ).options( description ).positional(p).run(), vm );
        po::notify(vm);
    }

    if ( vm.count( "help" ) ) {
        std::cerr << description << std::endl;
        return 0;
    }
    const size_t col_x = vm["x"].as<size_t>();
    const size_t col_y = vm["y"].as<size_t>();

    std::vector< std::pair<double, double> > xyValues;

    for ( const auto& file: vm[ "input-files" ].as<std::vector< std::string > >() ) {
        std::ifstream in( file );
        adportable::csv::list_string_type alist; // pair<variant,string>
        adportable::csv::csv_reader reader;
        if ( vm.count( "skip" ) ) {
            for ( size_t n = 0; n < vm[ "skip" ].as< size_t >(); ++n ) {
                if ( !( reader.read( in, alist ) && in.good() ) )
                    return 1;
            }
        }
        while ( reader.read( in, alist ) && in.good() ) {
            if ( alist.size() > std::max(col_x, col_y) ) {
                auto x = boost::apply_visitor( visitor(), std::get< 0 >( alist[ col_x ] ) );
                auto y = boost::apply_visitor( visitor(), std::get< 0 >( alist[ col_y ] ) );
                xyValues.emplace_back( x, y );
            }
        }

        auto xy_functor = [xyValues](size_t idx){return xyValues.at(idx);};

        if ( vm.count( "fit" ) && vm["fit"].as<std::string>() == "nd") {
            if ( auto res = fit_normal_distribution<double>( xy_functor, xyValues.size() ) ) {
                auto [mu, sigma, A] = *res;
                std::cout << "Fitted normal: mu=" << mu << ", sigma=" << sigma << ", A=" << A << "\n";
            } else {
                ADDEBUG() << "Fit failed.\n";
            }
        } else {
            const size_t degree = vm["degree"].as<size_t>();

            if (auto res = adportable::PolFit<double>::fit(xy_functor, xyValues.size(), degree)) {
                auto [coeffs, chisqr] = *res;
                ADDEBUG() << "Coefficients:";
                for (auto c : coeffs)
                    std::cout << c << ", ";
                ADDEBUG() << "\nChi-squared: " << chisqr;
            } else {
                ADDEBUG() << "Fit failed.\n";
            }
        }

        // Eigen::Matrix< double, 2, Eigen::Dynamic > xyValues(2, values.size() );
        // for ( const auto& value: values ) {
        //     ADDEBUG() << value;
        //     xyValues << std::get<0>(value), std::get<1>(value);
        // }
//        ADDEBUG() << xyValues;
        // ADDEBUG() << xyValues;
    }

}
