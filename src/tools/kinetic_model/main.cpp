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

#include <iostream>
#include <vector>
#include <boost/numeric/odeint.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

typedef std::vector<double> state_type;

double k1 = 1.0;   // A + OH- → A- + H2O
double k2 = 0.5;   // A + H3O+ → AH+ + H2O
double kr = 0.02;  // AH+ + A- → 2A
double k3 = 0.01;  // AH+ + OH- → A + H2O
double k4 = 0.01;  // A- + H3O+ → A + H2O

// Indices: 0:A, 1:AH+, 2:A-, 3:H3O+, 4:OH-
void reaction(const state_type &y, state_type &dy, double /* t */) {
    double A    = y[0];
    double AHp  = y[1];
    double Am   = y[2];
    double H3Op = y[3];
    double OHm  = y[4];

    dy[0] = -k1*A*OHm - k2*A*H3Op + kr*AHp*Am + k3*AHp*OHm + k4*Am*H3Op;
    dy[1] =  k2*A*H3Op - kr*AHp*Am - k3*AHp*OHm;
    dy[2] =  k1*A*OHm - kr*AHp*Am - k4*Am*H3Op;
    dy[3] = -k2*A*H3Op - k4*Am*H3Op;
    dy[4] = -k1*A*OHm - k3*AHp*OHm;
}

// Observer for printing
struct push_back_state {
    std::vector<std::tuple<double, state_type>> &states_;

    push_back_state(std::vector<std::tuple<double, state_type> > &s) : states_(s) {}

    void operator()(const state_type &x, double t) {
        states_.emplace_back(t, x);
    }
};


struct helium_range {
    std::optional< std::tuple< double, double, double > >
    operator()( const std::vector< double >& v ) const {
        if ( v.size() == 2 )
            return std::make_tuple( v.at(0), v.at(1), (v.at(1)-v.at(0))/10 );
        if ( v.size() >= 3 )
            return std::make_tuple( v.at(0), v.at(1), v.at(2) );
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
            ( "helium", po::value< double > ()->default_value(1.0),  "Initial regent ion conc." )
            ( "analyte", po::value< double > ()->default_value(1.0),  "Initial analyte conc." )
            ( "k1", po::value< double > ()->default_value(0.2),  "A + OH- -> A- + H2O." )
            ( "k2", po::value< double > ()->default_value(0.02),  "A + H3O- -> AH- + H2O." )
            ( "kr", po::value< double > ()->default_value(0.00), "AH+ + A- -> 2A" )
            ( "k3", po::value< double > ()->default_value(0.00), "AH+ + OH- -> A + H2O" )
            ( "k4", po::value< double > ()->default_value(0.00), "A- + H3O+ -> A + H2O" )
            ( "helium_range", po::value< std::vector<double> > ()->multitoken(),  "He start,end,step." )
            ;
        po::store( po::command_line_parser( argc, argv).options( description ).run(), vm);
        po::notify(vm);
    }
    if ( vm.count( "help" ) ) {
        std::cerr << description << std::endl;
        return 0;
    }

    k1 = vm["k1"].as<double>(); // A + OH- → A- + H2O
    k2 = vm["k2"].as<double>(); // A + H3O+ → AH+ + H2O
    kr = vm["kr"].as<double>(); // AH+ + A- → 2A
    k3 = vm["k3"].as<double>(); // AH+ + OH- → A + H2O
    k4 = vm["k4"].as<double>(); // A- + H3O+ → A + H2O

    // Initial concentrations: [A, AH+, A-, H3O+, OH-]
    // state_type y0 = {1.0, 0.0, 0.0, 0.50, 0.5};
    state_type y0 = {
        vm["analyte"].as<double>()
        , 0.0
        , 0.0
        , vm["helium"].as<double>()
        , vm["helium"].as<double>()
    };

    if ( vm.count("helium_range") ) {
        const auto& v = vm["helium_range"].as< std::vector< double > >();
        if ( auto range = helium_range()( vm["helium_range"].as< std::vector< double > >() ) ) {
            auto [beg,end,step] = *range;
            std::cerr << "helium_range = " << beg << ", " << end << ", " << step << std::endl;


            // Print header
            std::cout << "regent,time,A,AH+,A-,H3O+,OH-\n";
            std::vector< std::tuple< double, state_type > > states; // time, state
            for ( auto it = beg; it <= end; it += step ) {
                states.clear();
                boost::numeric::odeint::runge_kutta4<state_type> stepper;
                size_t steps = boost::numeric::odeint::integrate_const(stepper
                                                                       , reaction
                                                                       , y0
                                                                       , 0.0
                                                                       , 5.0
                                                                       , 0.001
                                                                       , push_back_state(states));
                const auto& [time, state] = states.back();
                std::cout << it << "," << time;
                for (double val : state)
                    std::cout << "," << val;
                std::cout << "\n";
            }
        }

    } else {
        std::vector<std::tuple<double, state_type> > states;

        boost::numeric::odeint::runge_kutta4<state_type> stepper;
        size_t steps = boost::numeric::odeint::integrate_const(stepper
                                                               , reaction
                                                               , y0
                                                               , 0.0
                                                               , 5.0
                                                               , 0.001
                                                               , push_back_state(states));

        // Print header
        std::cout << "time,A,AH+,A-,H3O+,OH-\n";
        for (size_t i = 0; i < steps; ++i) {
            auto [time, state] = states[i];
            std::cout << time;
            for (double val : state)
                std::cout << "," << val;
            std::cout << "\n";
        }
    }
    return 0;
}
