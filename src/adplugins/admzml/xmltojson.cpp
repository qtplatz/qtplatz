/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "xmltojson.hpp"
#include <adportable/debug.hpp>
#include <pugixml.hpp>
#include <boost/json.hpp>
#include <boost/spirit/home/x3/support/unused.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/variant.hpp>
#include <map>
#include <iostream>

namespace mzml {

    namespace x3 = boost::spirit::x3;

    using typed_value = boost::variant<double, int, std::string>;

    inline typed_value parse_typed_value(std::string_view input) {
        typed_value result;
        auto it = input.begin();
        auto end = input.end();

        // Try to parse as double, then int, then fallback to string
        auto parser = x3::double_ | x3::int_ | x3::lexeme[+x3::char_];

        bool success = x3::parse(it, end, parser, result);

        if (!success || it != end) {
            // fallback as raw string (if partial parse or no match)
            return std::string(input);
        }
        return result;
    }

    boost::json::value
    to_value::operator()(const pugi::xml_node& node) const
    {
        boost::json::object json_obj;

        // Add attributes
        for (auto attr : node.attributes()) {
            auto val = parse_typed_value(attr.value());
            boost::apply_visitor([&](const auto& v) {
                json_obj["@" + std::string(attr.name())] = v;
            }, val);
        }

        // Map of name -> array of children
        std::map< std::string, std::vector<boost::json::value> > children;

        for ( pugi::xml_node child : node.children() ) {
            if (child.type() != pugi::node_element)
                continue;
            children[ child.name() ].emplace_back((*this)(child) );
        }

        // Add children to JSON
        for (auto& [key, vec] : children) {
            if (vec.size() == 1)
                json_obj[key] = vec.front();
            else
                json_obj[key] = boost::json::value_from(vec);
        }

        return json_obj;
    }


} // namespace
