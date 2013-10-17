// formula_parser.cpp : Defines the entry point for the console application.
//

#if defined _MSC_VER
#pragma warning(disable:4100)
#endif

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <boost/format.hpp>

namespace client {
    namespace qi = boost::spirit::qi;

    const char * element_table [] = {
        "H",                                                                                                            "He",
        "Li", "Be",                                                                       "B",  "C",  "N",  "O",  "F",  "Ne", 
        "Na", "Mg",                                                                       "Al", "Si", "P",  "S",  "Cl", "Ar",
        "K",  "Ca", "Sc", "Ti", "V",  "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As", "Se", "Br", "Kr",  
        "Rb", "Sr", "Y",  "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn", "Sb", "Te", "I",  "Xe",  
        "Cs", "Ba", "Lu", "Hf", "Ta", "W",  "Re", "Os", "Ir", "Pt", "Au", "Hg", "Tl", "Pb", "Bi", "Po", "At", "Rn",
        "Fr", "Ra", "Ac", "Th", "Pa", "U",  "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No", "Lr",
        // Lanthanoids
        "La", "Ce", "Pr", "Nd", "Pm", "Sm",  "Eu", "Gd",  "Tb", "Dy",  "Ho", "Er" "Tm", "Yb",
        // Actinoids
        "Ac", "Th", "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No"
    };

    typedef std::pair< size_t, const char * > atom_type;
    typedef std::map< atom_type, size_t > map_type;
    
    void map_add( map_type& m, const std::pair<const atom_type, std::size_t>& p ) {
        m[ p.first ] += p.second;
    }

    void map_join( map_type& m, map_type& a ) {
        for( map_type::value_type& p: a )
            m[ p.first ] += p.second;
    }

    void map_mul( map_type& m, std::size_t n ) {
        for( map_type::value_type& p: m )
            p.second *= n;
    }

    using boost::phoenix::bind;
    using boost::phoenix::ref;
    using boost::spirit::qi::_val;
    using boost::spirit::ascii::space;
    using boost::spirit::ascii::space_type;
    using boost::spirit::qi::_1;

    template<typename Iterator>
    struct chemical_formula_parser : boost::spirit::qi::grammar< Iterator, map_type() > {

        chemical_formula_parser() : chemical_formula_parser::base_type( molecule )
                                  , element( element_table, element_table )  {
            molecule =
				+ (
                    atoms            [ boost::phoenix::bind(&map_add, _val, qi::_1) ]
                    | repeated_group [ boost::phoenix::bind(&map_join, _val, qi::_1 ) ]
                    | space
                    )
                ;
            atoms = 
                atom >> ( qi::uint_ | qi::attr(1u) ) // default to 1
                ;
            atom =
                ( qi::uint_ | qi::attr(0u) ) >> element
                ;
            repeated_group %= // forces attr proparation
                '(' >> molecule >> ')'
                    >> qi::omit[ qi::uint_[ boost::phoenix::bind( map_mul, qi::_val, qi::_1 ) ] ]
                ;
        }

        qi::rule<Iterator, atom_type() > atom;
        qi::rule<Iterator, std::pair< atom_type, std::size_t >() > atoms;
        qi::rule<Iterator, map_type()> molecule, repeated_group;
        qi::symbols<char, const char *> element;
    };
}

int
main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    std::string str;
    
    typedef std::string::const_iterator iterator_type;

    client::chemical_formula_parser< iterator_type > cf;

    while (std::getline(std::cin, str))  {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

        client::map_type map;
        iterator_type it = str.begin();
        iterator_type end = str.end();

        if ( boost::spirit::qi::parse( it, end, cf, map ) && it == end ) {
            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded\n";
            std::cout << str << " Parses OK: " << std::endl;
            std::cout << str << " map size: " << map.size() << std::endl;

            for ( auto e: map ) {
                std::cout << "<sup>" << e.first.first << "</sup>"
                          << e.first.second
                          << "<sub>" << e.second << "</sub>" << std::endl;
            }
            std::cout << "-------------------------\n";
        } else {
            std::cout << "-------------------------\n";
            std::cout << "Parsing failed for\n";
            std::cout << str;
            std::cout << "-------------------------\n";
        }
    }
    std::cout << "Bye... :-) \n\n";
	return 0;
}

