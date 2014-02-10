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

namespace std {
    // override '<' in order to sort elements (atom_type below) in alphabetical order
    bool operator < ( const std::pair<int, const char *>& lhs, const std::pair<int, const char *>& rhs ) {
		return std::strcmp( lhs.second, rhs.second ) < 0 || lhs.first < rhs.first;
    }
}

namespace client {
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

    typedef std::pair< int, const char * > atom_type;
    typedef std::map< atom_type, size_t > map_type;

    struct formulaComposition {
        static void formula_add( map_type& m, const std::pair<const atom_type, std::size_t>& p ) {
            m[ p.first ] += p.second;
        }
        
        static void formula_join( map_type& m, map_type& a ) {
            for( map_type::value_type& p: a )
                m[ p.first ] += p.second;
        }
        
        static void formula_repeat( map_type& m, std::size_t n ) {
            for( map_type::value_type& p: m )
                p.second *= n;
        }
    };

    static const char * braces [] = { "(", ")" };

    typedef std::vector< std::pair< atom_type, size_t > > format_type;

    struct formulaFormat {
        static void formula_add( format_type& m, const std::pair<const atom_type, std::size_t>& p ) {
            m.push_back( p );
        }
        
        static void formula_join( format_type& m, format_type& a ) {
            m.push_back( std::make_pair( atom_type( 0, braces[0] ), 0 ) );
            for ( auto t: a )
                m.push_back( t );
        }
        
        static void formula_repeat( format_type& m, std::size_t n ) {
            m.push_back( std::make_pair( atom_type( 0, braces[1] ), n ) );
        }
    };

    template<typename Iterator, typename handler, typename startType>
    struct chemical_formula_parser : boost::spirit::qi::grammar< Iterator, startType() > {

        chemical_formula_parser() : chemical_formula_parser::base_type( molecule ), element( element_table, element_table )  {
            molecule =
				+ (
                    atoms            [ boost::phoenix::bind(&handler::formula_add, _val, qi::_1) ]
                    | repeated_group [ boost::phoenix::bind(&handler::formula_join, _val, qi::_1 ) ]
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
                    >> qi::omit[ qi::uint_[ boost::phoenix::bind( handler::formula_repeat, qi::_val, qi::_1 ) ] ]
                ;
        }

        qi::rule<Iterator, atom_type() > atom;
        qi::rule<Iterator, std::pair< atom_type, std::size_t >() > atoms;
        qi::rule<Iterator, startType()> molecule, repeated_group;
        qi::symbols<char, const char *> element;
    };

}

int
main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    std::string str;
    std::wstring wstr;
    
    client::chemical_formula_parser< std::string::const_iterator, client::formulaComposition, client::map_type > cf;
    client::chemical_formula_parser< std::wstring::const_iterator, client::formulaFormat, client::format_type > wcf;
    // client::chemical_formula_parser< std::wstring::const_iterator, client::formulaComposition, client::map_type > wcf2;

    while (std::getline(std::cin, str))  {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;
        do {
            client::map_type map;
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
        } while(0);

        do {
            wstr.resize( str.size() );
            std::copy( str.begin(), str.end(), wstr.begin() );
            std::wcout << "as wide: " << wstr << std::endl;

            client::format_type fmt;
            std::wstring::const_iterator it = wstr.begin();
            std::wstring::const_iterator end = wstr.end();

            if ( boost::spirit::qi::parse( it, end, wcf, fmt ) && it == end ) {
                std::cout << "-------------------------\n";
                std::cout << str << " Parses OK: " << std::endl;
                for ( auto e: fmt ) {
                    if ( std::strcmp( e.first.second, "(" ) == 0 )
                        std::cout << "(";
                    else if ( std::strcmp( e.first.second, ")" ) == 0 )
                        std::cout << ")" << e.second;
                    else {
                        if ( e.first.first )
                            std::cout << "<sup>" << e.first.first << "</sup>";
                        std::cout << e.first.second;
                        if ( e.second > 1 )
                            std::cout << "<sub>" << e.second << "</sub>";
                    }
                }
                std::cout << "\n-------------------------\n";
            } else {
                std::cout << "-------------------------\n";
                std::cout << "Parsing failed for\n";
                std::cout << str;
                std::cout << "-------------------------\n";
            }
        } while(0);
    }

    std::cout << "Bye... :-) \n\n";
	return 0;
}

