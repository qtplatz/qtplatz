// formula_parser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <map>

namespace client {
    namespace qi = boost::spirit::qi;

    const wchar_t * element_table [] = {
        L"H",                                                                                                                  L"He",
        L"Li", L"Be",                                                                       L"B",  L"C",  L"N",  L"O",  L"F",  L"Ne", 
        L"Na", L"Mg",                                                                       L"Al", L"Si", L"P",  L"S",  L"Cl", L"Ar",
        L"K",  L"Ca", L"Sc", L"Ti", L"V",  L"Cr", L"Mn", L"Fe", L"Co", L"Ni", L"Cu", L"Zn", L"Ga", L"Ge", L"As", L"Se", L"Br", L"Kr",  
        L"Rb", L"Sr", L"Y",  L"Zr", L"Nb", L"Mo", L"Tc", L"Ru", L"Rh", L"Pd", L"Ag", L"Cd", L"In", L"Sn", L"Sb", L"Te", L"I",  L"Xe",  
        L"Cs", L"Ba", L"Lu", L"Hf", L"Ta", L"W",  L"Re", L"Os", L"Ir", L"Pt", L"Au", L"Hg", L"Tl", L"Pb", L"Bi", L"Po", L"At", L"Rn",
        L"Fr", L"Ra", L"Ac", L"Th", L"Pa", L"U",  L"Np", L"Pu", L"Am", L"Cm", L"Bk", L"Cf", L"Es", L"Fm", L"Md", L"No", L"Lr",
        // Lanthanoids
        L"La", L"Ce", L"Pr", L"Nd", L"Pm", L"Sm",  L"Eu", L"Gd",  L"Tb", L"Dy",  L"Ho", L"Er" L"Tm", L"Yb",
        // Actinoids
        L"Ac", L"Th", L"Pa", L"U", L"Np", L"Pu", L"Am", L"Cm", L"Bk", L"Cf", L"Es", L"Fm", L"Md", L"No"
    };

    typedef std::map< const wchar_t *, std::size_t > map_type;

    void map_add( map_type& m, const std::pair<const wchar_t *, std::size_t>& p ) {
        m[ p.first ] += p.second;
    }

    void map_join( map_type& m, map_type& a ) {
        BOOST_FOREACH( map_type::value_type& p, a )
            m[ p.first ] += p.second;
    }

    void map_mul( map_type& m, std::size_t n ) {
        BOOST_FOREACH( map_type::value_type& p, m )
            p.second *= n;
    }

    template<typename Iterator>
    struct chemical_formula_parser : boost::spirit::qi::grammar< Iterator, map_type() > {

        chemical_formula_parser() : chemical_formula_parser::base_type( molecule ), element( element_table, element_table )  {
            using boost::phoenix::bind;
            using boost::spirit::qi::_val;
            using boost::spirit::ascii::space;

            molecule =
                +(
                atoms          [ bind(&map_add, _val, qi::_1) ]
            | repeated_group [ bind(&map_join, _val, qi::_1 ) ] 
            | space
                )
                ;
            atoms = 
                element >> ( qi::uint_ | qi::attr(1u) ) // default to 1
                ;
            repeated_group %= // forces attr proparation
                '(' >> molecule >> ')'
                >> qi::omit[ qi::uint_[ bind( map_mul, qi::_val, qi::_1 ) ] ]
            ;
        }
        qi::rule<Iterator, std::pair< const wchar_t *, std::size_t >() > atoms;
        qi::rule<Iterator, map_type()> molecule, repeated_group;
        qi::symbols<wchar_t, const wchar_t *> element;
    };
}

int
_tmain(int argc, _TCHAR* argv[])
{
    std::string str;

    typedef std::wstring::const_iterator iterator_type;

    client::chemical_formula_parser< iterator_type > cf;

    while (std::getline(std::cin, str))  {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

        client::map_type map;
        std::wstring formula;
        struct copy {
            std::wstring& formula_;
            copy( std::wstring& formula ) : formula_(formula) {}
            void operator () ( int c ) { formula_ += c; };
        };
        std::for_each( str.begin(), str.end(), copy( formula ) );

        iterator_type it = formula.begin();
        iterator_type end = formula.end();
        if ( boost::spirit::qi::parse( it, end, cf, map ) && it == end ) {
            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded\n";
            std::cout << str << " Parses OK: " << std::endl;

            BOOST_FOREACH( client::map_type::value_type& p, map )
                std::wcout << p.first << ": " << p.second << std::endl;
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

