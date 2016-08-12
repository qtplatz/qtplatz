// formula_parser.cpp : Defines the entry point for the console application.
//

#if defined _MSC_VER
#pragma warning(disable:4100)
#endif

#include <compiler/workaround.h>
#include <compiler/disable_unused_parameter.h>
#include <adportable/formula_parser.hpp>
#include <boost/format.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <map>

namespace parser {
    template< typename char_type > bool
    parse( typename std::basic_string< char_type >::const_iterator& it
           , const typename std::basic_string< char_type >::const_iterator end
           , adportable::chem::icomp_type& comp ) {

        comp = adportable::chem::icomp_type(); // clear

        typedef typename std::basic_string< char_type >::const_iterator iterator_type;    
    
        using adportable::chem::formulaComposition;
        using adportable::chem::icomp_type;
        adportable::chem::chemical_formula_parser< iterator_type, formulaComposition, icomp_type > cf;
        return boost::spirit::qi::parse( it, end, cf, comp );
    }
}

namespace formatter {

    static const char * braces [] = { "(", ")" };
    static const char * separators [] = { "+", "-" };
    using adportable::chem::atom_type;
        
    typedef std::vector< std::pair< atom_type, size_t > > format_type;
    typedef std::pair< format_type, int > iformat_type;

    struct formulaFormat {

        static void formula_add( iformat_type& m, const std::pair<const atom_type, std::size_t>& p ) {
            m.first.emplace_back( p );
        }

        static void formula_join( iformat_type& m, iformat_type& a ) {

            if ( a.second )
                m.second += a.second; // charge_group
            else
                m.first.emplace_back( atom_type( 0, braces[0] ), 0 ); // repeat_group

            for ( auto t: a.first )
                m.first.emplace_back( t );
            
        }

        static void formula_repeat( iformat_type& m, std::size_t n ) {
            m.first.emplace_back( atom_type( 0, braces[1] ), n );
        }
        static void charge_state( iformat_type& m, adportable::chem::charge_type& c ) {
            m.second += ( c.second == '+' ? c.first : -c.first );
        }
    };

    template< typename char_type > static bool
    format( typename std::basic_string< char_type >::const_iterator& it
            , const typename std::basic_string< char_type >::const_iterator end, iformat_type& fmt ) {

        typedef typename std::basic_string< char_type >::const_iterator iterator_type;
        adportable::chem::chemical_formula_parser< iterator_type, formulaFormat, iformat_type > cf;
        
        return boost::spirit::qi::parse( it, end, cf, fmt );
    }    
}


int
main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    std::string str;
    static std::string delimiters = "+-";

    namespace chem = adportable::chem;
    namespace qi = boost::spirit::qi;

    while (std::getline(std::cin, str))  {
        
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

        do {
            adportable::chem::icomp_type comp;

            std::basic_string< char >::const_iterator it = str.begin();
            //auto it = str.begin();

            size_t count(0);
            std::cout << "Parsing\t'" << str << "'" << std::endl;

            while ( parser::parse<char>( it, str.end(), comp ) ) {
                std::cout << "---------- " << count++ << " ---------------\n";
                std::cout << "\tParsing succeeded for '" << str.substr( 0, it - str.begin() ) << "'\n";
                std::cout << str << " map size: " << comp.first.size() << std::endl;
                
                for ( auto e: comp.first )
                    std::cout << "\t" << e.first.first
                              << " "  << e.first.second
                              << " " << e.second << std::endl;

                std::cout << "\tCharge state: " << comp.second << std::endl;
                std::cout << "-------------------------\n";

                if ( it == str.end() || std::find( delimiters.begin(), delimiters.end(), *it ) == delimiters.end() )
                    break;

                std::cout << "\tseparator: " << *it++
                          << "\tremains '" << str.substr( it - str.begin() ) << "'" << std::endl;
            }

            if ( it != str.end() ) {
                std::cout << "-------------------------\n";
                std::cout << "Parsing failed at " << *it << std::endl;
                std::cout << "-------------------------\n";
            }
        } while(0);

        do {
            formatter::iformat_type fmt;
            std::vector< std::string > list;
            
            size_t count(0);
            std::basic_string< char >::const_iterator it = str.begin();
            //auto it = str.begin();
            
            while ( formatter::format<char>( it, str.end(), fmt ) ) {

                if ( it == str.end() || std::find( delimiters.begin(), delimiters.end(), *it ) == delimiters.end() )
                    break;
                
                auto sep = std::find_if( formatter::separators, formatter::separators + 2, [&]( const char * a ){ return *it == *a; } );
                if ( sep != formatter::separators + 2 ) {
                    fmt.first.emplace_back( adportable::chem::atom_type( 0, *sep ), 0 );
                }
                ++it;
            }
            
            if ( it != str.end() )
                std::cout << "Parse error: ";
            
            if ( fmt.second ) // has charge
                std::cout << "[";
            for ( auto e: fmt.first )
                std::cout << "\"" << e.first.first << " " << e.first.second << " " << e.second << "\", ";
            if ( fmt.second )  // has charge
                std::cout << "]" << std::abs( fmt.second ) << ( fmt.second > 0 ? '+' : '-' );
        } while ( 0 );

        std::cout << std::endl;
    }
    std::cout << "Bye... :-) \n\n";
	return 0;
}

