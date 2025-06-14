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

#include <iostream>
#include <pugixml.hpp>
#include "xmlwalker.hpp"
#include "accession.hpp"

namespace mzml {

    xmlWalker::xmlWalker( const std::string& tab
                          , std::ostream& out ) : tab_( tab )
                                                , out_( out )
    {
    }

    void
    xmlWalker::operator()( const pugi::xml_node& node ) const
    {
        for ( auto node1: node.select_nodes( "./*[not(self::cvParam)][not(self::offset)]" ) ) {
            std::map< std::string, std::string > attrs;
            for ( auto attr: node1.node().select_nodes( "@*" ) )
                attrs[ attr.attribute().name() ] = attr.attribute().value();
            mzml::accession ac( node );
            out_ << tab_ << node1.node().name();
            if ( not attrs.empty() ) {
                for ( const auto& attr: attrs )
                    out_ << " " << attr.first << "\"=\"" << attr.second << "\"";
            }
            if ( not ac.empty() )
                out_ << tab_ << node1.node().name() << "\t[" << ac.toString() << "]";
            out_ << std::endl;
            xmlWalker{ tab_ + "\t" }( node1.node() );
        }
    }
} // namespace
