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

#include "timesquaredscanlaw.hpp"
#include <adportable/debug.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <utility>

namespace {
    class serializer {
        std::ostream& o_;
        template<class Tuple, std::size_t... Is>
        void serializer_tuple_impl(const Tuple& t, std::index_sequence<Is...>){
            (((*this) << (Is == 0 ? "" : ", ") << std::get<Is>(t)), ...);
        }
    public:
        serializer( std::ostream& o ) : o_( o ) {}

        template<typename T> serializer& operator << ( const T& t ) { o_ << t; return *this; }
        serializer& operator<<( std::ostream&(*f)(std::ostream&) ) { o_ << f; return *this; } // endl

        template<typename... Args> serializer& operator << ( const std::tuple< Args...>& t ) {
            (*this) << "{";
            serializer_tuple_impl( t, std::index_sequence_for<Args...>{});
            return (*this) << "}";
        }
    };


}

namespace po = boost::program_options;

struct arg_to_double {
    // return mass, charge, formula
    std::optional< std::tuple< double, int, std::string > > operator()( const std::string& arg ) {
        char * end {};
        double value = std::strtod( arg.c_str(), &end );
        if ( end != arg.c_str() && *end == '\0' )
            return {{ value, 0, {} }};
        using adcontrols::ChemicalFormula;
        auto [m,charge] = ChemicalFormula().getMonoIsotopicMass( ChemicalFormula::split( arg) );
        if ( m != 0 )
            return {{ m, charge, arg }};
        return {};
    }
};

int
main(int argc, char *argv[])
{
    po::variables_map vm;
    po::options_description description( argv[0] );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "args",           po::value< std::vector< std::string > >(),             "formulae|times" )
            ( "formulae",       po::value< bool >()->default_value( true ),          "formulae|masses|times" )
            ( "accelerator,v",  po::value< double >()->default_value( 7000 ),        "accelerator voltage (V)" )
            ( "length,L",       po::value< double >()->default_value( 1.0 ),         "flight length (m)" )
            ( "tdelay",         po::value< double >()->default_value( 0.0 ),         "t0 (s)" )
            ( "query",          po::value< std::string >()->default_value( "time" ), "time|mass|voltage" )
            ( "velocity",       po::value< double >(), "velocity (m/s) to compute ion energy" )
            ;

        po::positional_options_description p;
        p.add( "args",  -1 );
        po::store( po::command_line_parser( argc, argv )
                   .options( description ).positional(p).run(), vm );
        po::notify(vm);
    }

    serializer out( std::cout );

    if ( vm.count( "help" ) || (vm.count( "args" ) == 0) ) {
        out << description;
        return 0;
    }

    adportable::TimeSquaredScanLaw scanlaw(
        vm[ "accelerator" ].as < double >()
        , vm[ "tdelay" ].as<double>()
        , vm[ "length" ].as< double >()
        );

    if ( vm.count("args") ) {
        for ( const auto& arg: vm[ "args" ].as< std::vector< std::string > >() ) {
            if ( auto kv = arg_to_double()( arg ) ) {
                out << *kv << "\t";
                if ( vm.count( "velocity" ) ) {
                    using namespace adportable;
                    double velocity = vm[ "velocity" ].as< double >(); // m/s
                    double m = std::get<0>(*kv) * kATOMIC_MASS_CONSTANT; // kg
                    double eV = (velocity*velocity)/(2*kELEMENTAL_CHARGE / m);

                    out << std::make_tuple("eV", eV);
                } else if ( vm[ "query" ].as< std::string >() == "time" ) {
                    double tof = scanlaw.getTime( std::get<0>(*kv), 0 );
                    out << std::make_tuple( "tof (s)", tof );
                    out << "\t" << std::make_tuple( "velocity (m/s)"
                                                    , scanlaw.fLength(0) / tof );
                }
                out << std::endl;
            }
        }
    }


}
