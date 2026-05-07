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

#include "msdata.hpp"
#include <boost/json.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/json/extract.hpp>
#include <adportable/debug.hpp>
#include <algorithm>
#include <iostream>
#include <span>

using namespace shrader;

msdata::~msdata()
{
}

msdata::msdata() : loaded_( false )
                 , data_{ 0 }
{
    static_assert(sizeof( detail::block ) == msdata::block_size
                  , "struct 'msdata' not alinged to 256 octets, check declaration.");
}

msdata::msdata( const msdata& t ) : loaded_( t.loaded_ )
                                  , data_( t.data_ )
{
}

bool
msdata::load( std::istream& in, size_t fsize, int64_t pos )
{
    if ( ( fsize - pos ) < block_size )
        return 0;

    int scan = 0;
    in.seekg( pos );

    while ( ( fsize - in.tellg() ) >= block_size ) {

        detail::block d;
        in.read( reinterpret_cast<char *>( &d ), block_size );
        if ( in.fail() ) {
            ADDEBUG() << "## Error: file read failed. ";
            return 0; // io error
        }
        if ( scan == 0 && ( ( d.flags & 0x0f) == record_type_code ) ) {
            scan = d.scan;
        }

        if ( ( (d.flags & 0x0f) != record_type_code ) || scan != d.scan ) {
            return data_.size();
        } else {
            data_.emplace_back( std::move( d ) );
        }
    }
    return data_.size();
}

int16_t
msdata::flags( size_t block ) const
{
    if ( data_.size() > block )
        return data_[ block ].flags;
    return 0;
}

int16_t
msdata::scan( size_t block ) const
{
    if ( data_.size() > block )
        return data_[ block ].scan;
    return -1;
}

int16_t
msdata::threshold( size_t block ) const
{
    if ( data_.size() > block )
        return data_[ block ].threshold;
    return -1;
}

int16_t
msdata::nions( size_t block ) const
{
    if ( data_.size() > block )
        return data_[ block ].nions;
    return 0;
}

int32_t
msdata::xlow( size_t block ) const
{
    if ( data_.size() > block )
        return data_[ block ].xlow;
    return -1;
}

int32_t
msdata::xhigh( size_t block ) const
{
    if ( data_.size() > block )
        return data_[ block ].xhigh;
    return -1;
}

size_t
msdata::size() const
{
    return data_.size();
}

std::vector< int32_t >
msdata::intensities() const
{
    auto inflate_block = []( const detail::block& b
                             , std::vector< int32_t>& y
                             , std::int64_t& pos
                             , bool& seen_index ){
        if (b.nions < 0 || b.nions > 60)
            throw std::runtime_error("invalid nions");
        auto encoded = std::span<const int32_t>(b.u.profile, static_cast<std::size_t>(b.nions) );
        for (int32_t v : encoded) {
            if ( v < 0 ) {
                pos = static_cast<std::int64_t>(-v) - 1; // 1-origin -> 0-origin
                if (pos < 0)
                    throw std::runtime_error("invalid index marker");

                if (static_cast<std::size_t>(pos) > y.size())
                    y.resize(static_cast<std::size_t>(pos), 0);

                seen_index = true;
            } else {
                if ( not seen_index)
                    throw std::runtime_error("profile value before index marker");

                if (static_cast<std::size_t>(pos) >= y.size())
                    y.resize(static_cast<std::size_t>(pos) + 1, 0);
                y[static_cast<std::size_t>(pos++)] = v;
            }
        }
    };

    std::vector< int32_t > y;
    std::int64_t pos = (-1);
    bool seen_index = false;

    for ( const auto& block: this->blocks() ) {
        inflate_block( block, y, pos, seen_index );
    }
    return y;
}

namespace shrader {
    namespace detail {
        void
        tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const block& t )
        {
            jv = {{ "block"
                    , {
                        { "scan", t.scan }
                        , { "flags", t.flags }
                        , { "threshold", t.threshold }
                        , { "nions", t.nions }
                        , { "xlow", t.xlow }
                        , { "u", boost::json::value_from( t.u.profile ) }
                    }
                }};
        }

        detail::block
        tag_invoke( const boost::json::value_to_tag< block >&, const boost::json::value& jv )
        {
            detail::block _;
            if ( jv.is_object() ) {
                using namespace adportable::json;
                auto obj = jv.as_object();
                // extract( obj,  reinterpret_cast< unsigned& >(_.algo_),       "algo" );
                // extract( obj,  _.threshold_, "threshold" );
                // extract( obj,  _.boundary_,  "boundary" );
                // extract( obj,  _.eq_,        "eq" );
            }

            return _;
        }

    } // details
}
