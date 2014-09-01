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

            text = lexeme[+(char_ - '>' - '<' - '$')   [_val += qi::_1] ]
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

            nodes =
                + ( node )
                >> *( qi::lit("$$$$") )
                ;
            
            nodes.name( "nodes" );
            node.name( "node" );
            start_tag.name( "start_tag" );
			text.name( "text" );
            
            qi::on_error<qi::fail> (
                nodes
				, std::cout << boost::phoenix::val( "Error! Expecting " )
                << _4
				<< boost::phoenix::val(" here: \"")
				<< boost::phoenix::construct< std::string >(_3, _2)
				<< boost::phoenix::val("\"")
                << std::endl
                );
        }

        qi::rule<Iterator, std::string()> text;
        qi::rule<Iterator, std::string()> start_tag;
        qi::rule<Iterator, std::pair< std::string, std::string>()> node;
        qi::rule<Iterator, nodes_type()> nodes;
    };

}

const char * text = 
">  <DSSTox_RID>75317 \n\
>  <DSSTox_CID>5 \n\
>  <DSSTox_Generic_SID>20005 \n\
>  <DSSTox_FileID>1_Tox21S_v2a\n\
>  <STRUCTURE_Formula>C2H5NO\n\
>  <STRUCTURE_MolecularWeight>59.0672\n\
>  <STRUCTURE_TestedForm_DefinedOrganic>parent\n\
>  <STRUCTURE_ChemicalType>defined organic\n\
>  <STRUCTURE_Shown>tested chemical\n\
>  <TestSubstance_ChemicalName>Acetamide\n\
>  <TestSubstance_CASRN>60-35-5\n\
>  <TestSubstance_Description>single chemical compound\n\
>  <ChemicalNote>blank\n\
>  <STRUCTURE_ChemicalName_IUPAC>acetamide\n\
>  <STRUCTURE_SMILES>CC(=O)\n\
>  <STRUCTURE_Parent_SMILES>CC(=O)N\n\
>  <STRUCTURE_InChIS>InChI=1S/C2H5NO/c1-2(3)4/h1H3,(H2,3,4)\n\
>  <STRUCTURE_InChIKey>DLFVBJFMPXGRIB-UHFFFAOYSA-N\n\
>  <Substance_modify_yyyymmdd>20080429\n\
>  <Note_Tox21S>blank\n$$$$";


int
main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    client::sdfile_parser< std::string::const_iterator > parser;
#if 1
    do {
        std::string str = text;

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
    } while (0);
#else
    std::string str;
    std::string line;
	while( std::getline( std::cin, line ) )
        str += line;

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
        
/*
	while( std::getline( std::cin, str ) ) {

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
*/
#endif
	return 0;
}

