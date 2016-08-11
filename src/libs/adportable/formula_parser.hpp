/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#if defined _MSC_VER
# pragma warning(disable:4100)
#endif

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iostream>

namespace adportable {

    namespace chem {

        namespace qi = boost::spirit::qi;
        using boost::phoenix::bind;
        using boost::spirit::qi::_val;
        using boost::spirit::qi::_1;
        using boost::spirit::ascii::space;

        const char * element_table [] = {
            "H",                                                                                                              "He",
            "Li", "Be",                                                                         "B",  "C",  "N",  "O",  "F",  "Ne", 
            "Na", "Mg",                                                                         "Al", "Si", "P",  "S",  "Cl", "Ar",
            "K",  "Ca", "Sc", "Ti", "V",  "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As", "Se", "Br", "Kr",  
            "Rb", "Sr", "Y",  "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn", "Sb", "Te", "I",  "Xe",  
            "Cs", "Ba", "Lu", "Hf", "Ta", "W",  "Re", "Os", "Ir", "Pt", "Au", "Hg", "Tl", "Pb", "Bi", "Po", "At", "Rn",
            "Fr", "Ra", "Ac", "Th", "Pa", "U",  "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No", "Lr",
            // Lanthanoids
            "La", "Ce", "Pr", "Nd", "Pm", "Sm",  "Eu", "Gd",  "Tb", "Dy",  "Ho", "Er" "Tm", "Yb",
            // Actinoids
            "Ac", "Th", "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No"
        };

        typedef std::pair< int, const char * > atom_type; // [isotope#]symbol; e.g. 13C
        typedef std::map< atom_type, int > comp_type;     // number of atoms; e.g. C6
        typedef std::pair< int, char > charge_type;       
        typedef std::pair< comp_type, int > icomp_type;

        struct formulaComposition {

            static void formula_add( icomp_type& m, const std::pair<const atom_type, int>& p ) {
                m.first[ p.first ] += p.second;
            }

            static void formula_join( icomp_type& m, icomp_type& a ) {
                std::for_each( a.first.begin(), a.first.end(), [&]( comp_type::value_type& p ){ m.first[ p.first ] += p.second; });
                m.second += a.second;
            }
        
            static void formula_repeat( icomp_type& m, int n ) {
                std::for_each( m.first.begin(), m.first.end(), [=]( comp_type::value_type& p ){ p.second *= n; });
            }

            static void charge_state( icomp_type& m, charge_type& c ) {
                m.second += ( c.second == '+' ? c.first : -c.first );
            }
        };

        template< typename Iterator, typename handler = formulaComposition, typename startType = icomp_type >
        struct chemical_formula_parser : boost::spirit::qi::grammar< Iterator, startType() > {

            chemical_formula_parser() : chemical_formula_parser::base_type( molecule )
                                      , element( element_table, element_table )  {
                molecule =
                    + (
                        atoms            [ boost::phoenix::bind( &handler::formula_add, _val, qi::_1 ) ]
                        | repeated_group [ boost::phoenix::bind( &handler::formula_join, _val, qi::_1 ) ]
                        | charged_group  [ boost::phoenix::bind( &handler::formula_join, _val, qi::_1 ) ]                        
                        | space
                        )
                    ;
                charged_group %=
                    '[' >> molecule >> ']'
                        >> qi::omit[ - ( charge_state [ boost::phoenix::bind(&handler::charge_state, _val, qi::_1 ) ] ) ]
                    ;
                charge_state =
                    ( qi::uint_ | qi::attr(1u) ) >> ( qi::char_( '+' ) | qi::char_( '-' ) )
                    ;
                atoms = 
                    atom >> ( qi::uint_ | qi::attr(1u) ) // default to 1
                    ;
                atom =
                    ( qi::uint_ | qi::attr(0u) ) >> element
                    ;
                repeated_group %= // forces attr proparation
                    '(' >> molecule >> ')'
                        >> qi::omit[ qi::uint_[ boost::phoenix::bind( handler::formula_repeat, qi::_val, qi::_1 ) ] ]
                    ;
            }

            qi::rule<Iterator, atom_type() > atom;
            qi::rule<Iterator, std::pair< atom_type, int >() > atoms;
            qi::rule<Iterator, charge_type() > charge_state;
            qi::rule<Iterator, startType() > molecule, repeated_group, charged_group;
            qi::symbols<char, const char *> element;
        };

    }
}

