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
#include <iostream>
#include <fstream>
#include <sstream>

namespace client {
    namespace qi = boost::spirit::qi;
    using boost::phoenix::bind;
    using boost::spirit::qi::_val;
    using boost::spirit::qi::_1;
    using boost::spirit::ascii::space;
    using boost::spirit::ascii::space_type;
    using boost::spirit::ascii::char_;
    using qi::lexeme;
    using boost::phoenix::at_c;
    using boost::phoenix::push_back;
    using namespace qi::labels;

    typedef std::pair< std::string, std::string > node_type;
    typedef std::vector< node_type > nodes_type;

    template<typename Iterator>
    struct sdfile_parser : boost::spirit::qi::grammar< Iterator, nodes_type() > {

        sdfile_parser() : sdfile_parser::base_type( nodes ) {

            text = lexeme[+(char_ - '<')   [_val += qi::_1] ]
                ;

            start_tag =
                '>'
                >> *(space)
                >> qi::lit('<')  
                >> lexeme[+(char_ - '>') [_val += qi::_1] ]
                >> '>' 
                ;
            
            node = 
                start_tag  [ at_c<0>(_val) = qi::_1 ]
                >> *(text  [ at_c<1>(_val) = qi::_1 ])
                ;

            nodes %=
                node
                | ( nodes >> node )
                | ( nodes >> qi::lit("$$$$") )
                ;
        }

        qi::rule<Iterator, std::string()> text;
        qi::rule<Iterator, std::string()> start_tag;
        qi::rule<Iterator, std::pair< std::string, std::string>()> node;
        qi::rule<Iterator, nodes_type()> nodes;
    };

}

int
main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    client::sdfile_parser< std::string::const_iterator > parser;

    std::string str;

    while ( std::getline( std::cin, str ) ) {

        std::string::const_iterator it = str.begin();
        std::string::const_iterator end = str.end();
        
        client::nodes_type v;
        if ( boost::spirit::qi::parse( it, end, parser, v ) && it == end ) {
            std::cout << "OK: ";
            for ( auto& node: v ) 
                std::cout << "[" << node.first << "]=" << node.second << std::endl;
        } else {
            std::cout << "-------------------------\n";
            std::cout << "Parsing failed\n";
            std::cout << "-------------------------\n";
        }
    }
	return 0;
}

