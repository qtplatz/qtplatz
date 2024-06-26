// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2021-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2021-2022 MS-Cheminformatics LLC
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

#include "json_helper.hpp"
#include "debug.hpp"
#include <boost/json.hpp>
#include <boost/optional.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <iomanip>
#include <vector>

namespace {
    struct tokenizer {
        std::vector< std::string > tokens_;
        tokenizer( std::string s ) {
            if ( s.at(0) == '/' )
                s.erase( 0, 1 ); // ignore leading '/'
            size_t pos(0);
            while ( ( pos = s.find_first_of( "./" ) ) != std::string::npos ) {
                tokens_.emplace_back( s.substr( 0, pos ) );
                s.erase( 0, pos + 1 );
            }
            if ( !s.empty() )
                tokens_.emplace_back( s );
        }
        std::vector< std::string >::const_iterator begin() const { return tokens_.begin(); }
        std::vector< std::string >::const_iterator end() const { return tokens_.end(); }
    };

    std::optional<uint32_t> find_number( const std::string& key ) {
        if ( !key.empty() && key.find_first_not_of( "0123456789" ) != std::string::npos )
            return {};
        try {  return std::stol( key ); } catch ( std::exception& ) { /**/ }
        return {};
    }
}


using namespace adportable;

// static
boost::json::value
json_helper::parse( const std::string& s )
{
    boost::system::error_code ec;
    auto jv = boost::json::parse( s, ec );
    if ( ! ec )
        return jv;
    return {};
}

boost::json::value
json_helper::parse( const boost::optional< std::string >& s )
{
    boost::system::error_code ec;
    if ( s ) {
        auto jv = boost::json::parse( *s, ec );
        if ( !ec )
            return jv;
    }
    return {};
}

// static
boost::json::value
json_helper::find( const boost::json::value& jv, const std::string& keys ) // dot delimited key-list
{
    tokenizer tok( keys );
    if ( auto p = if_contains( jv, keys ) ) {
        return *p;
    }
    return {};
}

// static
const boost::json::value *
json_helper::if_contains( const boost::json::value& jv, const std::string& keys ) // dot delimited key-list
{
    tokenizer tok( keys );
    const boost::json::value * pv = &jv;
    for ( const auto& key: tok ) {
        if ( !pv ) {
            return nullptr;
        } else if ( pv->kind() == boost::json::kind::object ) {
            pv = pv->as_object().if_contains( key );
        } else if ( pv->kind() == boost::json::kind::array ) {
            if ( auto index = find_number( key ) ) {
                if ( pv->as_array().size() > *index ) {
                    pv = &(pv->as_array().at( *index ));
                } else {
                    return nullptr;
                }
            }
        }
    }
    return pv;
}

const boost::json::value *
json_helper::find_pointer( const boost::json::value& jv, const std::string& keys, boost::system::error_code& ec ) // JSON pointer (RFC 6901)
{
    ec = {};
#if BOOST_VERSION >= 108100
    return jv.find_pointer( keys, ec );
#else
    return if_contains( jv, keys );
#endif
}

boost::json::value
json_helper::find( const std::string& s, const std::string& keys ) // dot delimited key-list
{
    boost::system::error_code ec;
    auto jv = boost::json::parse( s, ec );
    if ( !ec ) {
        return find( jv, keys );
    }
    return {};
}

boost::json::value
json_helper::find( const boost::optional< std::string >& s, const std::string& keys ) // dot delimited key-list
{
    return s ? find( *s, keys ) : boost::json::value{};
}

namespace adportable {

    template<> ADPORTABLESHARED_EXPORT boost::optional< boost::uuids::uuid >
    json_helper::value_to( const boost::json::value& jv, const std::string& keys )
    {
        auto t = find( jv, keys );
        if ( !t.is_null() )
            return boost::lexical_cast< boost::uuids::uuid >( boost::json::value_to< std::string >( t ) );
        return {};
    }

    //---------------------

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const boost::uuids::uuid& t )
    {
        jv = boost::uuids::to_string( t );
    }


    boost::uuids::uuid
    tag_invoke( const boost::json::value_to_tag< boost::uuids::uuid>&, const boost::json::value& jv )
    {
        return boost::lexical_cast< boost::uuids::uuid >( jv.as_string().data() );
    }
}
