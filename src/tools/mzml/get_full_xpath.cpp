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

#include <pugixml.hpp>

std::string
get_full_xpath(pugi::xml_node node)
{
    std::string path;
    while (node && node.type() == pugi::node_element) {
        std::string name = node.name();
        // Optional: include index if parent has multiple with same name
        int index = 1;
        pugi::xml_node sibling = node.previous_sibling(name.c_str());
        while (sibling) {
            ++index;
            sibling = sibling.previous_sibling(name.c_str());
        }
        path = "/" + name + "[" + std::to_string(index) + "]" + path;
        node = node.parent();
    }
    return path.empty() ? "/" : path;
}
