// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2018 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@qtplatz.com
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

#include "boost_json.hpp"
#include "data.hpp"
#include <boost/json.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/lexical_cast.hpp>
#include <array>
#include <type_traits>
#include <iostream>

namespace {

    template<bool = true >
    struct workaround {
        template< typename T > void assign( T& t, boost::json::string_view value ) {
            t = boost::lexical_cast< T >( value );
        }
    };

    template<>
    struct workaround< false > {
        template< typename T > void assign( T& t, boost::json::string_view value ) {}
    };

    template< typename T >
    void ptree_extract( const boost::json::object& obj, std::vector< T >& t, boost::string_view key ) {
        if ( obj.at( key ).is_array() ) {
            for ( const auto& item: obj.at( key ).as_array() ) {
                T d;
                workaround<true>().assign( d, item.as_string() );
                t.emplace_back( d );
            }
        }
    }

    // ----------- ordinary extract ----------
    template<class T>
    inline void extract( const boost::json::object& obj, T& t, boost::json::string_view key )  {
        try {
            t = boost::json::value_to<T>( obj.at( key ) );
        } catch ( std::exception& ex ) {
            if ( obj.at( key ).is_string() ) {
                workaround< std::is_arithmetic< T >::value >().assign( t, obj.at( key ).as_string() );
            } else {
                BOOST_THROW_EXCEPTION(std::runtime_error( std::string( "exception at value_to(" ) + key.data() + ")"));
            }
        }
    }
    // template<> void extract( const boost::json::object& obj, boost::uuids::uuid& t, boost::json::string_view key );
}

namespace tick {
    namespace hv {
        inline void tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const value& t )
        {
            jv = {{ "id", t.id }
                , { "name", t.name }
                , { "sn",   t.sn }
                , { "set",  t.set }
                , { "act",  t.act }
                , { "unit",  t.unit }
            };
        }

        inline value tag_invoke( boost::json::value_to_tag< value >&, const boost::json::value& jv )
        {
            value t;
            try {
                auto obj = jv.as_object();
                extract( obj, t.id, "id" );
                extract( obj, t.name, "name" );
                extract( obj, t.sn,   "sn" );
                extract( obj, t.set,  "set" );
                extract( obj, t.act,  "act" );
                extract( obj, t.unit, "unit" );
            } catch ( std::exception& ex ) {
                BOOST_THROW_EXCEPTION( ex );
            }
            return t;
        }
    }

    void tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const adc& t )
    {
        jv = {{ "tp", t.tp }
            , { "nacc", t.nacc }
            , { "values", t.values }
        };
    }

    adc tag_invoke( boost::json::value_to_tag< adc >&, const boost::json::value& jv )
    {
        adc t;
            const auto& obj = jv.as_object();
            extract( obj, t.tp, "tp" );
            extract( obj, t.nacc, "nacc" );
        try {
            extract( obj, t.values, "values" );
        } catch ( std::exception& ex ) {
            // maybe property_tree::ptree generated json -- apply workaround
            ptree_extract( obj, t.values, "values" );
        }
        return t;
    }
}

void tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const data& t )
{
    jv = { { "tick"
            , {    { "tick", t.tick }
                 , { "time", t.time }
                 , { "nsec", t.nsec }
                 , { "hv"
                       , {{ "values", t.values }
                         ,{ "alarms"
                              , {{ "alarm", {{ "text", t.alarm }} }}
                        }}}
                 , { "adc", t.adc }
            }
        }
    };
}

data
tag_invoke( boost::json::value_to_tag< data >&, const boost::json::value& jv )
{
    data t;
    if ( jv.is_object() ) {
        if ( auto tick = jv.as_object().if_contains( "tick" ) ) {
            const auto& obj = tick->as_object();
            extract( obj, t.tick, "tick" );
            extract( obj, t.time, "time" );
            extract( obj, t.nsec, "nsec" );
            if ( auto hv = obj.if_contains( "hv" ) ) {
                try {
                    extract( hv->as_object(), t.values, "values" );
                    if ( auto alarms = hv->as_object().if_contains( "alarms" ) ) {
                        if ( auto alarm = alarms->as_object().if_contains( "alarm" ) ) {
                            extract( alarm->as_object(), t.alarm, "text" );
                        }
                    }
                } catch ( std::exception& ex ) {
                    BOOST_THROW_EXCEPTION(ex);
                }
            }
            extract( obj, t.adc, "adc" );
        }
    }
    return t;
}


struct boost_json::impl {
    boost::json::storage_ptr sp_;
    boost::json::value jtop_;
    unsigned char buffer_[ 8192 ];
    impl() : sp_( boost::json::make_shared_resource< boost::json::monotonic_resource >() ) {
    }

    static impl& instance()  {
        static impl _;
        return _;
    }
};


boost_json::boost_json()
{
}

boost_json::~boost_json()
{
}

bool
boost_json::parse( const std::string& json_string )
{
    boost::system::error_code ec;

    boost::json::monotonic_resource mr( impl::instance().buffer_ );
    // impl::instance().jtop_ = boost::json::parse( json_string, ec, &mr );
    impl::instance().jtop_ = boost::json::parse( json_string, ec );
    return !ec;
}

std::string
boost_json::stringify( const boost::json::value& d, bool pritty )
{
    return boost::json::serialize( d );
}

std::string
boost_json::stringify( bool ) const
{
    return boost::json::serialize( impl::instance().jtop_ );
}

bool
boost_json::map( data& d )
{
    d = boost::json::value_to< data >( impl::instance().jtop_ );
    return true;
}

std::string
boost_json::make_json( const data& d )
{
    auto jv = boost::json::value_from( d, impl::instance().sp_ );
    return boost::json::serialize( jv );
}
