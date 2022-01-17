/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/map.hpp>

namespace adchem {

    namespace qi = boost::spirit::qi;
    using boost::phoenix::bind;
    using boost::spirit::qi::_val;
    using boost::spirit::qi::_1;
    using boost::spirit::ascii::space;
    using boost::spirit::ascii::space_type;
    using qi::standard::char_;   // <-- using boost::spirit::ascii::char_;
    using qi::lexeme;
    using boost::phoenix::at_c;
    using boost::phoenix::push_back;
    using namespace qi::labels;

    typedef std::pair< std::string, std::string > node_type;
    typedef std::vector< node_type > nodes_type;

    template<typename Iterator>
    struct sdfile_parser : boost::spirit::qi::grammar< Iterator, nodes_type() > {

        sdfile_parser() : sdfile_parser::base_type( nodes ) {

            text = lexeme[+(char_ - '>' - '<' - '$') [_val += qi::_1] ]
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
