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

#include "lrphead2.hpp"
#include <adportable/json_helper.hpp>
#include <adportable/json/extract.hpp>
#include <adportable/debug.hpp>
#include <boost/json.hpp>
#include <boost/algorithm/string.hpp>
#include <cstddef>
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

lrphead2::lrphead2() : loaded_( false )
                     , data_{ 0 }
{
}

lrphead2::lrphead2( const lrphead2& t ) : loaded_( t.loaded_ )
                                        , data_( t.data_ )
{
}

bool
lrphead2::load( std::istream& in, size_t fsize )
{
    if ( ( fsize - in.tellg() ) >= data_size ) {
        in.read( data_.data(), data_.size() );
        if ( !in.fail() )
            loaded_ = true;
    }
    return loaded_;
}

int32_t
lrphead2::flags() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header2, flags ));
}

std::string
lrphead2::descline1() const
{
    auto a = std::string( data_.data() + offsetof( detail::header2, descline1 ), 80 );
    std::erase( a, '\0' );
    return boost::trim_copy( a );
}

std::string
lrphead2::descline2() const
{
    auto a = std::string( data_.data() + offsetof( detail::header2, descline2 ), 80 );
    std::erase( a, '\0' );
    return boost::trim_copy( a );
}

std::string
lrphead2::client() const
{
    auto a = std::string( data_.data() + offsetof( detail::header2, client ), 40 );
    std::erase( a, '\0' );
    return boost::trim_copy( a );
}

const int32_t *
lrphead2::istd() const
{
    return reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header2, istd ));
}

namespace shrader {
    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const lrphead2& t )
    {
        auto p = reinterpret_cast< const detail::header2 *>( t.data_.data() );
        jv = {{ "header2"
                    , {
                    { "flags",            p->flags }
                    , { "descline1",       t.descline1()  }
                    , { "descline2",       t.descline2()  }
                    , { "client",          t.client()  }
                    , { "istd",           boost::json::value_from( p->istd ) }
                    , { "tail",           p->tail }
                }
            }};
    }

    lrphead2
    tag_invoke( const boost::json::value_to_tag< lrphead2 >&, const boost::json::value& jv )
    {
        lrphead2 _;
        auto p = reinterpret_cast< detail::header2 *>( _.data_.data() );
        if ( jv.is_object() ) {
            using namespace adportable::json;
            auto obj = jv.as_object();
            extract( obj, p->flags, "flags" );
            // extract( obj, p->descline1, "descline1" );
            // extract( obj, p->descline2, "descline2" );
            // extract( obj, p->client, "client" );
            // extract( obj, p->istd, "istd" );
            extract( obj, p->tail, "tail" );
        }

        return _;
    }
}
