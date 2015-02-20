/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "lrphead2.hpp"
#include <istream>

namespace shrader {
    namespace detail {
#pragma pack(1)
        struct header2 {
            int32_t flags;
            char descline1[ 80 ];
            char descline2[ 80 ];
            char client[ 40 ];
            int32_t istd[ 13 ];
            int32_t tail;
        };
#pragma pack()

    }
}

using namespace shrader;

lrphead2::~lrphead2()
{
}

lrphead2::lrphead2(std::istream& in, size_t fsize) : loaded_( false )
{
    if ( ( fsize - in.tellg() ) >= data_size ) {
        in.read( data_.data(), data_.size() );
        if ( !in.fail() )
            loaded_ = true;
    }
}

int32_t 
lrphead2::flags() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header2, flags ));
}

std::string 
lrphead2::descline1() const
{
    return std::string( data_.data() + offsetof( detail::header2, descline1 ), 80 );
}

std::string 
lrphead2::descline2() const
{
    return std::string( data_.data() + offsetof( detail::header2, descline2 ), 80 );
}

std::string 
lrphead2::client() const
{
    return std::string( data_.data() + offsetof( detail::header2, client ), 40 );
}

const int32_t *
lrphead2::istd() const
{
    return reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header2, istd ));
}


