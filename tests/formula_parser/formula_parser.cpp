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

#include <adportable/formula_parser.hpp>

namespace test {
        
    // for chemical formula formatter
    static const char * braces [] = { "(", ")" };
    using adportable::chem::atom_type;
    
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
}

int
main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    std::string str;
    std::wstring wstr;

    namespace chem = adportable::chem;
    namespace qi = boost::spirit::qi;

    typedef 
        chem::chemical_formula_parser < std::string::const_iterator
                                     , chem::formulaComposition
                                     , chem::comp_type> composition_calculator_t;
    
    typedef 
        chem::chemical_formula_parser < std::wstring::const_iterator
                                     , test::formulaFormat
                                     , test::format_type > formula_formatter_t;

    while (std::getline(std::cin, str))  {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

        // compute compsition (ascii input)
        do {
            chem::comp_type comp;
            auto it = str.begin();
            auto end = str.end();
            
            if ( qi::parse( it, end, composition_calculator_t(), comp ) && it == end ) {
                std::cout << "-------------------------\n";
                std::cout << "Parsing succeeded\n";
                std::cout << str << " Parses OK: " << std::endl;
                std::cout << str << " map size: " << comp.size() << std::endl;
                
                for ( auto e: comp )
                    std::cout << e.first.first << " "  << e.first.second  << " " << e.second << std::endl;
                std::cout << "-------------------------\n";
            } else {
                std::cout << "-------------------------\n";
                std::cout << "Parsing failed\n";
                std::cout << "-------------------------\n";
            }
        } while(0);

        // pritty formatted text (wchar_t input)
        do {
            wstr.resize( str.size() );
            std::copy( str.begin(), str.end(), wstr.begin() );
            std::wcout << "as wide: " << wstr << std::endl;

            test::format_type fmt;
            auto it = wstr.begin();
            auto end = wstr.end();

            if ( qi::parse( it, end, formula_formatter_t(), fmt ) && it == end ) {

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

