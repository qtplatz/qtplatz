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

#include "tableofelement.hpp"
#include "element.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <cctype>
#include <algorithm>

#if defined _MSC_VER
# pragma warning(disable:4100)
#endif

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/tuple.hpp>
#include <map>

typedef std::pair< element, int > element_type; // symbol, number of element, ex. C2 (two carbon atoms)

namespace detail {

    const char * element_dictionary [] = { "C", "H", "N", "O", "S", "P", "Cl" }; // just for quick test for peptides

    typedef std::map< const char *, int > comp_type;

    void comp_add( comp_type& comp, const std::pair< const char *, int >& c ) {
        comp[ c.first ] += c.second;
    }
    void comp_join( comp_type& comp, comp_type& a ) {
        for ( auto& c: a )
            comp[ c.first ] += c.second;
    }
    void comp_repeat( comp_type& comp, int n ) {
        for ( auto& c: comp )
            c.second *= n;
    }

    namespace qi = boost::spirit::qi;
    using boost::phoenix::bind;
    using boost::spirit::qi::_val;
    using boost::spirit::qi::_1;
    using boost::spirit::ascii::space;

    template<typename Iterator = std::string::const_iterator>
    struct chemical_formula_parser : boost::spirit::qi::grammar<Iterator, comp_type() > {
        
        chemical_formula_parser() : chemical_formula_parser::base_type( molecule )
                                  , element( element_dictionary, element_dictionary )  {
            molecule =
                + (
                    atoms            [ boost::phoenix::bind(&comp_add, _val, qi::_1) ]
                    | repeated_group [ boost::phoenix::bind(&comp_join, _val, qi::_1 ) ]
                    | qi::space
                    )
                ;
            // remark: this grammer does not handle isotopes, (ex. "18O" does recognize as heavy oxygen)
            atoms = 
                element >> ( qi::uint_ | qi::attr(1u) ) // default to 1, 
                ;
            repeated_group %= // forces attr proparation
                '(' >> molecule >> ')'
                    >> qi::omit[ qi::uint_[ boost::phoenix::bind( comp_repeat, qi::_val, qi::_1 ) ] ]
                ;
        }
        qi::rule<Iterator, std::pair< const char *, int >() > atoms;
        qi::rule<Iterator, comp_type()> molecule, repeated_group;
        qi::symbols<char, const char *> element;
    };
}

bool
scanner( std::string& line, std::vector< element_type >& mol )
{
    std::string::const_iterator it = line.begin();
    std::string::const_iterator end = line.end();

    detail::chemical_formula_parser< std::string::const_iterator > parser;
/*
    detail::comp_type composition;
    if ( boost::spirit::qi::parse( it, end, parser, composition ) && it == end ) {
        
    }
*/  
    return true;
}

int
main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    std::string line;

    while ( std::getline( std::cin, line ) ) {
        if ( line.empty() || line[ 0 ] == 'q' || line[0] == 'Q')
            break;

        std::vector< element_type > mol;
        scanner( line, mol );
    }
}
