/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <adportable/moment.hpp>
#include <adportable/spectrum_processor.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/math/distributions/normal.hpp>
#include <cmath>
#include <fstream>
#include <random>
#include <ratio>
#include <iomanip>
#include <iostream>

namespace po = boost::program_options;

int centroid_error( boost::math::normal_distribution<>&, double, double
                    , std::normal_distribution<>&, std::mt19937&, bool random );

int area( boost::math::normal_distribution<>&, double, double );

int
main(int argc, char *argv[])
{
    po::variables_map vm;
    po::options_description description( argv[0] );
    {
        description.add_options()
            ( "help,h",       "Display this help message" )
            ( "profile",      "print profile data for gnuplot" )
            ( "fraction,a",   "print fraction at peak time, with offset" )
            ( "replicates,r", po::value< size_t >()->default_value( 20 ), "replicates" )
            ( "fence",        "fence" )
            ( "width",        po::value< double >()->default_value( 4  ),  "peak width (ns)" )
            ( "time",         po::value< double >()->default_value( 10 ),  "peak time (us)" )
            ( "rate",         po::value< double >()->default_value( 2  ),  "sampling rate (GS/s)" )
            ( "offset",       po::value< double >()->default_value( 0  ),  "sampling offset (ns)" )
            ( "step",         po::value< double >()->default_value( 0.2  ),"replicates step (ns)" )
            ;

        po::positional_options_description p;
        p.add( "args",  -1 );
        po::store( po::command_line_parser( argc, argv ).options( description ).positional(p).run(), vm );
        po::notify(vm);
    }

    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    double interval = 1.0 / ( vm[ "rate" ].as< double >() * std::giga::num );
    double m = vm[ "time" ].as< double >() / std::micro::den;
    double s = vm[ "width" ].as< double >() / std::nano::den;

    boost::math::normal_distribution<> nd( m, s / 2.0 );

    double x0 = m - interval * 50;

    double max_y = boost::math::pdf( nd, m ) * 1.1;

    std::vector< double > x, y;
    for ( int i = 0; i < 128; ++i ) {
        auto xx = x0 + i * interval;
        x.emplace_back( xx );
        y.emplace_back( boost::math::pdf( nd, xx ) / max_y );
    }

    if ( vm.count( "fraction" ) ) {
        double w = vm[ "width" ].as< double >() / std::nano::den;
        double step = vm[ "step" ].as< double >() / std::nano::den;
        size_t replicates = vm[ "replicates" ].as< size_t >();

        double centre = m;
        auto a0 = adportable::spectrum_processor::area( adportable::spectrum_processor::getFraction( x.data(), x.size(), m - (w/2), m + (w/2) )
                                                        , 0.0, y.data(), y.size() );

        do {
            double left = centre - (w/2);
            double right = centre + (w/2);
            auto afrac = adportable::spectrum_processor::getFraction( x.data(), x.size(), left, right );
            auto area  = adportable::spectrum_processor::area( afrac, 0.0, y.data(), y.size() );
            std::cout << boost::format("%d,\t%.14le,\t%d,\t%.14le,\t%.14le,\t%.14le,\t%.14le,\t%g")
                % afrac.lPos % afrac.lFrac % afrac.uPos % afrac.uFrac % area % (left*std::micro::den) % (right*std::micro::den) % (area / a0) << std::endl;
            centre += step;
        } while ( replicates-- > 0 );
        return 0;
    }
    if ( vm.count( "fence" ) ) {
        double w = vm[ "width" ].as< double >() / std::nano::den;
        double step = vm[ "step" ].as< double >() / std::nano::den;
        size_t replicates = vm[ "replicates" ].as< size_t >();

        double centre = m;
        centre += step * replicates;

        double left = centre - (w/2);
        double right = centre + (w/2);

        auto maxIt = std::max_element( y.begin(), y.end() );

        std::cout << boost::format("%.14le,\t%.14le") % (left*std::micro::den) % *maxIt << std::endl;
        std::cout << boost::format("%.14le,\t%.14le") % (right*std::micro::den) % *maxIt << std::endl;

        return 0;
    }

    for ( int i = 0; i < y.size(); ++i )
        std::cout << boost::format( "%.14le,\t%.14le" ) % (x[i]*std::micro::den) % y[i] << std::endl;
}
