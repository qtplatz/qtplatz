/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <boost/json.hpp>
#include <pugixml.hpp>
#include <map>

namespace mzml {

    class to_value {
    public:
        boost::json::value operator()(const pugi::xml_node& node) const {
            boost::json::object json_obj;

            // Add attributes
            for (auto attr : node.attributes())
                json_obj["@" + std::string(attr.name())] = attr.value();

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
    };

} // namespace
