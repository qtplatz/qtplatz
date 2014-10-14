/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/molecule.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <chrono>
#include <cstring>
#include <numeric>

bool
scanner( std::string& line, adcontrols::mol::molecule& mol )
{
    return adcontrols::ChemicalFormula::getComposition( mol.elements, line );
}

int
main(int argc, char * argv[])
{
    adcontrols::isotopeCluster cluster;

    double threshold_daltons = cluster.threshold_daltons();

    if ( --argc ) {
        ++argv;
        while( argc-- ) {
            if ( std::strcmp( *argv, "-d" ) == 0 && argc ) {
                --argc;
                ++argv;
                threshold_daltons = atof( *argv ) / 1000;
                cluster.threshold_daltons( threshold_daltons );
            }
        }
    }
    std::cout << boost::format( "Set daltons threshold to: %gmDa\n" ) % ( cluster.threshold_daltons() * 1000);

    std::string line;
    std::cerr << "Enter formula: ";
    while (std::getline(std::cin, line))  {
            
        if (line.empty() || line[0] == 'q' || line[0] == 'Q')
            break;
            
        adcontrols::mol::molecule mol;
        if ( scanner( line, mol ) ) {
            std::cout << "formula: ";
            std::for_each( mol.elements.begin(), mol.elements.end(), [&]( const adcontrols::mol::element& e ){
                    std::cout << e.symbol() << e.count() << " ";
                });
            std::cout << std::endl;

            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

            cluster( mol );

            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            
            auto it = std::max_element( mol.cluster.begin(), mol.cluster.end()
                                        , [](const adcontrols::mol::isotope& a, const adcontrols::mol::isotope& b){
                                            return a.abundance < b.abundance;});
            const double pmax = it->abundance;
            const double sum = std::accumulate( mol.cluster.begin(), mol.cluster.end(), 0.0
                                                , []( double x, const adcontrols::mol::isotope& a ){ return x + a.abundance; } );

            int idx = 0;
            for ( auto& i: mol.cluster ) {
                double ratio = i.abundance / pmax * 100.0;
                if ( ratio >= 0.05 )
                    std::cout << boost::format( "[%2d] %.8f\t%.8e\t%.6f\n" ) % idx++ % i.mass % i.abundance % ratio;
            }
            std::cout << "processed "
                      << mol.cluster.size() << " peaks in: "
                      << double(std::chrono::duration_cast< std::chrono::microseconds >( end - start ).count())/1000 << "ms."
                      << std::endl;
            std::cout << boost::format( "sum =%.14f" ) % sum << std::endl;
        } else {
            std::cout << "Parse failed: " << line << std::endl;
        }
        std::cerr << "Enter formula: ";
    }

    std::cout << "Bye... :-) \n\n";
    return 0;
}

