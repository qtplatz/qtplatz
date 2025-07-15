// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include "helper.hpp"
#include <pugixml.hpp>
#include <adportable/bzip2.hpp>
#include <adportable/debug.hpp>
#include <adfs/sqlite.hpp>
#include <sstream>

namespace mzml {

    std::string to_string( const pugi::xml_node& node ) {
        std::ostringstream o;
        node.print( o );
        return  o.str();
    }

    template<> std::string archive_to_string( const pugi::xml_node& node, bool compress ) {
        std::string xml = to_string( node );
        if ( not compress )
            return xml;
        return bzip2_compress( std::move( xml ) );
    }

    std::string bzip2_compress( std::string&& data ) {
        std::string compressed;
        adportable::bzip2::compress( compressed, data.c_str(), data.size() );
        return compressed;
    }

    std::string bzip2_decompress( const std::string& compressed ) {
        if ( adportable::bzip2::is_a( compressed.data(), compressed.size() ) ) {
            std::string inflated;
            adportable::bzip2::decompress( inflated, compressed.data(), compressed.size() );
            return inflated;
        }
        return compressed;
    }

    std::string bzip2_decompress( adfs::blob blob ) {
        return bzip2_decompress( std::string{ reinterpret_cast< const char * >(blob.data()), blob.size() } );
    }

} // namespace mzml
