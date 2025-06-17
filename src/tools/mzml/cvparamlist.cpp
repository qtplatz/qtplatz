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

#include "cvparamlist.hpp"
#include "iostream"
#include <algorithm>
#include <map>

bool cvParamList::operator()( const std::vector< std::string >& files ) const
{
    std::map< std::string, std::string > cvParam;
    for ( auto file: files ) {
        pugi::xml_document doc;
        if ( auto result = doc.load_file( file.c_str() ) ) {
            for (auto param : doc.select_nodes("//cvParam")) {
                std::string name = param.node().attribute("name").value();
                cvParam[ param.node().attribute("accession").value() ] = param.node().attribute("name").value();
            }
        }
    }

    std::cout << "\nnamespace mzml {" << std::endl
              << "\tenum Accession {" << std::endl;

    for ( const auto& param: cvParam ) {
        std::string tag = param.first;
        std::replace( tag.begin(), tag.end(), ':', '_');
        if ( param == *cvParam.begin() )
            std::cout << std::format( "\t {}", tag) << std::endl;
        else
            std::cout << std::format( "\t, {}", tag) << std::endl;
    }
    std::cout << std::format( "\t, MS_MAX") << std::endl
              << "\t};" << std::endl
              << "} // namespace" << std::endl;

    std::cout << "\nnamespace mzml {" << std::endl
              << "\tconst std::array< std::tuple< std::string, Accession, std::string >, MS_MAX > accession_list = {{" << std::endl;

    for ( const auto& param: cvParam ) {
        std::string tag = param.first;
        std::replace( tag.begin(), tag.end(), ':', '_');
        std::cout << "\t,{" << std::format( "\t\"{}\",\t{},\t\"{}\"\t", param.first, tag, param.second) << "}" << std::endl;
    }
    std::cout << "\t}};" << std::endl
              << "} // namespace" << std::endl;
    return 0;
}
