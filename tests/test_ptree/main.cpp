// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/exception/all.hpp>
#include <iostream>

static void
print( const boost::property_tree::ptree& pt )
{
    using boost::property_tree::ptree;

    ptree::const_iterator end = pt.end();
    for (ptree::const_iterator it = pt.begin(); it != end; ++it) {
        std::cout << it->first << ": " << it->second.get_value<std::string>() << std::endl;
        print(it->second);
    }    
}

int
main()
{
    using namespace boost::property_tree;

    boost::property_tree::ptree pt;
    //xml_parser::read_xml( "csdebug.xml", pt );
    try {
        xml_parser::read_xml( "chemical-structure.txt", pt );
    } catch ( boost::exception& ex ) {
        std::cout << boost::diagnostic_information(ex) << std::endl;
    }

    // xml_parser::write_xml( std::cout, pt, boost::property_tree::xml_writer_make_settings<std::string>( ' ', 2 ) );
    std::cout << "==================" << std::endl;
    print( pt );
    std::cout << "==================" << std::endl;
    //std::cout << "------------------" << std::endl;
    //print( pt.get_child( "soap:Envelope.soap:Body.GetAsyncSearchResultResponse.GetAsyncSearchResultResult" ) );
    //std::cout << "------------------" << std::endl;

    // int i = 0;
    // for( auto& child : pt.get_child( "soap:Envelope.soap:Body.GetAsyncSearchResultResponse.GetAsyncSearchResultResult" ) ) {
    //     std::cout << "[" << i++ << "] " << child.second.data() << std::endl;
    // }

}
