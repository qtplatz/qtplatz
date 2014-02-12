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

#include "formula_parser.hpp"
#include "element.hpp"
#include "molecule.hpp"
#include "isotope.hpp"
#include <iostream>
#include <boost/format.hpp>

bool
scanner( std::string& line, molecule& mol )
{
    std::string::const_iterator it = line.begin();
    std::string::const_iterator end = line.end();

    chem::comp_type elemental_composition;
    chem::chemical_formula_parser< std::string::const_iterator > parser;
    if ( boost::spirit::qi::parse( it, end, parser, elemental_composition ) && it == end ) {
        for ( auto& atom: elemental_composition ) {
            const char * symbol = atom.first.second;
            // put result in alphabetical order
            auto it = std::lower_bound( mol.elements.begin(), mol.elements.end(), symbol, [](const molecule::element& lhs, const char * symbol ){
                    return std::strcmp( lhs.symbol, symbol ) < 0;
                });
            mol.elements.insert( it, molecule::element( atom.first.second, atom.second ) );
        }
        return true;
    }
    return false;
}

int
main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    std::string line;

    while (std::getline(std::cin, line))  {

        if (line.empty() || line[0] == 'q' || line[0] == 'Q')
            break;

        molecule mol;
        if ( scanner( line, mol ) ) {
            std::cout << "formula: ";
            std::for_each( mol.elements.begin(), mol.elements.end(), [&]( const molecule::element& e ){
                    std::cout << e.symbol << e.count << " ";
                });
            std::cout << std::endl;

            isotope::compute( mol );
            
            int idx = 0;
            std::for_each( mol.isotopes.begin(), mol.isotopes.end(), [&]( const molecule::isotope& i ){
                    std::cout << boost::format( "[%2d] %.8f\t%.8f\n" ) % idx++ % i.mass % i.abundance;
                });
            
        } else {
            std::cout << "Parse failed: " << line << std::endl;
        }
    }
    std::cout << "Bye... :-) \n\n";
	return 0;
}

