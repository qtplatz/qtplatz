/**************************************************************************
** Copyright (C) 2010-2026 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2026 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "lrphead3.hpp"
#include <boost/algorithm/string.hpp>
#include <cstddef>
#include <istream>

namespace shrader {
    namespace detail {
#pragma pack(1)
        struct header3 {
            int32_t flags;
            char instaltitle[ 80 ];
            char inlet[ 30 ];
            char comments[ 142 ];
        };
#pragma pack()
    }
}

using namespace shrader;

lrphead3::~lrphead3()
{
}

lrphead3::lrphead3() : loaded_( false )
                       , data_{ 0 }
{
}

lrphead3::lrphead3( const lrphead3& t ) : loaded_( t.loaded_ )
                                        , data_( t.data_ )
{
}

bool
lrphead3::load(std::istream& in, size_t fsize)
{
    if ( ( fsize - in.tellg() ) >= data_size ) {
        in.read( data_.data(), data_.size() );
        if ( !in.fail() )
            loaded_ = true;
    }
    return loaded_;
}

int32_t
lrphead3::flags() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header3, flags ));
}

std::string
lrphead3::instaltitle() const
{
    auto a = std::string( data_.data() + offsetof( detail::header3, instaltitle ), 80 );
    std::erase( a, '\0' );
    return boost::trim_copy( a );
}

std::string
lrphead3::inlet() const
{
    auto a = std::string( data_.data() + offsetof( detail::header3, inlet ), 80 );
    std::erase( a, '\0' );
    return boost::trim_copy( a );
}

std::string
lrphead3::comments() const
{
    auto a = std::string( data_.data() + offsetof( detail::header3, comments ), 142 );
    std::erase( a, '\0' );
    return boost::trim_copy( a );
}

namespace shrader {
    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const lrphead3& t )
    {
        auto p = reinterpret_cast< const detail::header3 *>( t.data_.data() );
        jv = {{ "header3"
                    , {
                    { "flags",            p->flags }
                    , { "instaltitle",      t.instaltitle().c_str() }
                    , { "inlet",            t.inlet().c_str() }
                    , { "comments",         t.comments().c_str() }
                    }
            }};
    }

}
