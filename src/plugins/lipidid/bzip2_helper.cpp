/**************************************************************************
** Copyright (C) 2010-2026 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2026 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "bzip2_helper.hpp"
#include <adportable/bzip2.hpp>

using namespace lipidid;

adfs::blob
bzip2_compress::operator()( const std::string& data ) const
{
    std::string bz2;
    adportable::bzip2::compress( bz2, data.data(), data.size() );
    return adfs::blob( bz2.size(), bz2.data() );
}

std::string
bzip2_decompress::operator()( const adfs::blob& blob ) const
{
    if ( adportable::bzip2::is_a( reinterpret_cast< const char * >(blob.data()), blob.size() ) ) {
        std::string data;
        adportable::bzip2::decompress( data, reinterpret_cast< const char *>(blob.data()), blob.size() );
        return data;
    } else {
        return { reinterpret_cast< const char *>(blob.data()), blob.size() };
    }
}
