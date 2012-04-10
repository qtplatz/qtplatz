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
#include <boost/fusion/include/std_pair.hpp>
#include <string>
#include <map>

using namespace fticr;

namespace fticr {  namespace client {

	namespace qi = boost::spirit::qi;

	typedef std::pair<std::string, std::string> pair_type;

	template<typename Iterator>
	struct jcampdx_parser : boost::spirit::qi::grammar< Iterator, pair_type() > {
/*
		jcampdx_parser() : jcampdx_parser::base_type( expr ) {
           expr = var_;
		}
*/
	};

}
}

jcampdxparser::jcampdxparser()
{
}

bool
jcampdxparser::parse_file( std::map< std::string, std::string >& map, const std::wstring& filename )
{
	return true;
}