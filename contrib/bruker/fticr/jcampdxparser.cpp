/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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
#if defined _MSC_VER
#pragma warning(disable:4100)
#endif
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#if defined _MSC_VER
#pragma warning(default:4100)
#endif
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <string>
#include <map>

using namespace fticr;

namespace fticr {  namespace client {

	namespace qi = boost::spirit::qi;

	typedef std::pair<std::string, std::string> pair_type;

	template<typename Iterator>
	struct jcampdx_parser : qi::grammar<Iterator, pair_type(), qi::ascii::space_type> {
		// this is not conformed JCAMP-DX parser, but only work for mass calibration data 
		// extract from Bruker's FT-ICR acqu file
		jcampdx_parser() : jcampdx_parser::base_type( pair ) {
			using boost::spirit::qi::_val;
			using boost::spirit::ascii::space;

			pair %=
				qi::lit("##") >> text >> '=' >> text //[ boost::phoenix::bind(&map_add, _val, qi::_1) ]
				;
			text %= +(qi::char_ - '=');  // terminate if '=' and space skipped
			// it might be necessary to use lexeme after '=' in order to avoid strip out spaces
			// text %= qi::lexeme[ +(qi::char_) ];
		}
		qi::rule<Iterator, pair_type(), qi::ascii::space_type> pair;
		qi::rule<Iterator, std::string(), qi::ascii::space_type> text;
	};
}
}

jcampdxparser::jcampdxparser()
{
}

bool
jcampdxparser::parse_file( std::map< std::string, std::string >& map, const std::wstring& filename )
{
	using boost::spirit::ascii::space;

	boost::filesystem::path path( filename );
	boost::filesystem::ifstream inf( path );
	if ( ! inf )
		return false;

	typedef std::string::const_iterator iterator_type;
	client::jcampdx_parser< iterator_type > grammer;

	std::string line;
	while ( std::getline( inf, line ) ) {
		iterator_type beg = line.begin();
		iterator_type end = line.end();
		std::pair< std::string, std::string > data;
		if ( boost::spirit::qi::phrase_parse( beg, end, grammer, space, data ) ) {
			map[ data.first ] = data.second;
		}
	}
	return true;
}
