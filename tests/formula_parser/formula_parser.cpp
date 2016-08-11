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


template< typename char_type >
static bool parse( const std::basic_string< char_type >& formula, adportable::chem::icomp_type& comp ) {
    using adportable::chem::formulaComposition;
    using adportable::chem::comp_type;
    using adportable::chem::icomp_type;
                
    typedef typename std::basic_string< char_type >::const_iterator iterator_type;
                
    adportable::chem::chemical_formula_parser< iterator_type, formulaComposition, icomp_type > cf;
    iterator_type it = formula.begin();
    iterator_type end = formula.end();

    return boost::spirit::qi::parse( it, end, cf, comp ) && it == end;
}

int
main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    std::string str;

    namespace chem = adportable::chem;
    namespace qi = boost::spirit::qi;

    // typedef 
    //     chem::chemical_formula_parser < std::string::const_iterator
    //                                     , chem::formulaComposition
    //                                     , chem::comp_type> composition_calculator_t;
    
    while (std::getline(std::cin, str))  {
        
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

        do {
            adportable::chem::icomp_type comp;

            if ( parse<char>( str, comp ) ) {
                std::cout << "-------------------------\n";
                std::cout << "Parsing succeeded\n";
                std::cout << str << " Parses OK: " << std::endl;
                std::cout << str << " map size: " << comp.first.size() << std::endl;
                
                for ( auto e: comp.first )
                    std::cout << e.first.first << " "  << e.first.second  << " " << e.second << std::endl;
                std::cout << "Charge state: " << comp.second.first << comp.second.second << std::endl;
                std::cout << "-------------------------\n";
            } else {
                std::cout << "-------------------------\n";
                std::cout << "Parsing failed\n";
                std::cout << "-------------------------\n";
            }
        } while(0);
    }
    std::cout << "Bye... :-) \n\n";
	return 0;
}

