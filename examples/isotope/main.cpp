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

int
main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    std::string str;
    std::wstring wstr;
    
    chem::chemical_formula_parser< std::string::const_iterator> cf;

    while (std::getline(std::cin, str))  {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

        chem::map_type map;
        std::string::const_iterator it = str.begin();
        std::string::const_iterator end = str.end();
        
        if ( boost::spirit::qi::parse( it, end, cf, map ) && it == end ) {
            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded\n";
            std::cout << str << " Parses OK: " << std::endl;
            std::cout << str << " map size: " << map.size() << std::endl;
            
            for ( auto e: map )
                std::cout << e.first.first << " "  << e.first.second  << " " << e.second << std::endl;
            std::cout << "-------------------------\n";
        } else {
            std::cout << "-------------------------\n";
            std::cout << "Parsing failed\n";
            std::cout << "-------------------------\n";
        }
    }
    std::cout << "Bye... :-) \n\n";
	return 0;
}

