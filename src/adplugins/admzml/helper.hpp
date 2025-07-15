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

#pragma once

#include <pugixml.hpp>
#include <sstream>
// #include <adfs/sqlite.hpp>

namespace adfs { class blob; }

namespace mzml {

    std::string to_string( const pugi::xml_node& node );
    std::string bzip2_compress( std::string&& );
    std::string bzip2_decompress( const std::string& compressed );
    std::string bzip2_decompress( adfs::blob );

    template< typename T >
    std::string archive_to_string( const T& t, bool compress ) {
        std::ostringstream ar;
        T::archive( ar, t );
        if ( not compress )
            return ar.str();
        return bzip2_compress( std::move( ar.str() ) );
    };

    template<> std::string archive_to_string( const pugi::xml_node& node, bool compress );
}
