/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "jcampdxparser.hpp"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
//#include <boost/spirit/include/phoenix_core.hpp>
//#include <boost/spirit/include/phoenix_operator.hpp>
//#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
//#include <boost/fusion/include/io.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <string>
#include <map>

using namespace fticr;

namespace fticr {  namespace client {

	namespace qi = boost::spirit::qi;

	typedef std::pair<std::string, std::string> pair_type;
	// typedef std::string pair_type;

	typedef std::map< std::string, std::size_t > map_type;

	void map_add( map_type& m, const std::pair<std::string, std::size_t>& p ) {
	}

	template<typename Iterator>
	struct jcampdx_parser : boost::spirit::qi::grammar< Iterator, map_type() > {

		jcampdx_parser() : jcampdx_parser::base_type( molecule ) {
			using boost::spirit::qi::_val;
			using boost::spirit::ascii::space;

			text2 =
				+(	text//    [ boost::phoenix::bind(&map_add, _val, qi::_1) ]
			     | space
				)
				;
			text = qi::lexeme[+(qi::char_)];
		}
		qi::rule<Iterator, std::string(), qi::ascii::space_type> text;
		qi::rule<Iterator, std::string(), qi::ascii::space_type> text2;
		qi::rule<Iterator, map_type()> molecule;
	};

/*
	template<typename Iterator>
	struct jcampdx_parser : qi::grammar< Iterator, pair_type(), qi::ascii::space_type > {

		jcampdx_parser() : jcampdx_parser::base_type( quoted_string ) {
			// using boost::phoenix::bind;
			using boost::spirit::qi::_val;
			using boost::spirit::ascii::space;

			quoted_string %= qi::lexeme ['"' +( qi::char_ - '"' ) >> '"' ]
			;
			//expr %= qi::lit("##") >> quoted_string >> '=' >> quoted_string
			//;
		}
		qi::rule<Iterator, std::string(), qi::ascii::space_type> quoted_string;
		qi::rule<Iterator, pair_type(), qi::ascii::space_type> expr;
	};
*/
}
}

jcampdxparser::jcampdxparser()
{
}

bool
jcampdxparser::parse_file( std::map< std::string, std::string >& map, const std::wstring& filename )
{
	(void)filename;
	using boost::spirit::ascii::space;

	typedef std::string::const_iterator iterator_type;

	client::jcampdx_parser< iterator_type > grammer;

	std::string s = "##\"$ABC\" = \"12345\"";
    iterator_type it = s.begin();
    iterator_type end = s.end();
       
	//std::pair< std::string, std::string > data;
	//bool r = boost::spirit::qi::parse( it, end, grammer, data );
	(void)map;
	//bool r = boost::spirit::qi::phrase_parse( it, end, grammer, space, map );
	return true;
}