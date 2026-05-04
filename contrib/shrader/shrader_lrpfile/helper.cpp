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
#include <adportable/base64.hpp>
#include <adportable/bzip2.hpp>
#include <adportable/debug.hpp>
#include <adfs/sqlite.hpp>
#include <sstream>

namespace shrader {

    std::string bzip2_compress( std::string&& data ) {
        std::string compressed;
        adportable::bzip2::compress( compressed, data.data(), data.size() );
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

    std::string block_to_string( const std::array< char, 256 >& data ) {
        return base64_encode( bzip2_compress( std::string( data.data(), data.size() ) ) );
    }

    std::string string_to_block( const std::string& data ) {
        return bzip2_decompress( base64_decode( data ) );
    }

} // namespace mzml
