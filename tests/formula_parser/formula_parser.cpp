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

#if 0
template< typename char_type > bool
parse( const std::basic_string< char_type >& formula, adportable::chem::icomp_type& comp ) {

    using adportable::chem::formulaComposition;
    using adportable::chem::icomp_type;
                
    typedef typename std::basic_string< char_type >::const_iterator iterator_type;
                
    adportable::chem::chemical_formula_parser< iterator_type, formulaComposition, icomp_type > cf;
    iterator_type it = formula.begin();
    iterator_type end = formula.end();

    if ( boost::spirit::qi::parse( it, end, cf, comp ) ) {
        //it == end;
        std::cout << "------ parse it=" << *it << " end=" << ( it == end ) << std::endl;
        return true;
    }
    return false;
}
#endif

template< typename char_type > bool
parse( typename std::basic_string< char_type >::const_iterator& it
       , const typename std::basic_string< char_type >::const_iterator end
       , adportable::chem::icomp_type& comp ) {

    comp = adportable::chem::icomp_type();

    typedef typename std::basic_string< char_type >::const_iterator iterator_type;    
    
    using adportable::chem::formulaComposition;
    using adportable::chem::icomp_type;
    adportable::chem::chemical_formula_parser< iterator_type, formulaComposition, icomp_type > cf;
    if ( boost::spirit::qi::parse( it, end, cf, comp ) ) {
        return true;
    }
    return false;
        
}

int
main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    std::string str;

    namespace chem = adportable::chem;
    namespace qi = boost::spirit::qi;

    while (std::getline(std::cin, str))  {
        
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

        do {
            adportable::chem::icomp_type comp;

            std::basic_string< char >::const_iterator it = str.begin();

            size_t count(0);
            
            while ( parse<char>( it, str.end(), comp ) ) {
                std::cout << "---------- " << count++ << " ---------------\n";
                std::cout << "Parsing succeeded\n";
                std::cout << str << " Parses OK: " << std::endl;
                std::cout << str << " map size: " << comp.first.size() << std::endl;
                
                for ( auto e: comp.first )
                    std::cout << "\t" << e.first.first << " "  << e.first.second  << " " << e.second << std::endl;
                std::cout << "\tCharge state: " << comp.second << std::endl;
                std::cout << "-------------------------\n";
                if ( it == str.end() )
                    break;
                std::cout << "---- separator: " << *it++ << std::endl;
            }
            if ( it != str.end() ) {
                std::cout << "-------------------------\n";
                std::cout << "Parsing failed at " << *it << std::endl;
                std::cout << "-------------------------\n";
            }
        } while(0);
    }
    std::cout << "Bye... :-) \n\n";
	return 0;
}

