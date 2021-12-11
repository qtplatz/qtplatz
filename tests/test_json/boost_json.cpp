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
#include <boost/system/error_code.hpp>
#include <boost/uuid/uuid.hpp>
#include <iostream>

namespace {
    template<class T>
    void extract( const boost::json::object& obj, T& t, boost::json::string_view key )  {
        try {
            t = boost::json::value_to<T>( obj.at( key ) );
        } catch ( std::exception& ex ) {
            BOOST_THROW_EXCEPTION(std::runtime_error("exception"));
        }
    }
    // template<> void extract( const boost::json::object& obj, boost::uuids::uuid& t, boost::json::string_view key );
}

namespace tick {
    namespace hv {
        void tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const value& t )
        {
            jv = {{ "id", t.id }
                , { "name", t.name }
                , { "sn",   t.sn }
                , { "set",  t.set }
                , { "act",  t.act }
                , { "unit",  t.unit }
            };
        }

        value tag_invoke( boost::json::value_to_tag< value >&, const boost::json::value& jv )
        {
            value t;
            auto obj = jv.as_object();
            extract( obj, t.id, "id" );
            extract( obj, t.name, "name" );
            extract( obj, t.sn,   "sn" );
            extract( obj, t.set,  "set" );
            extract( obj, t.act,  "act" );
            extract( obj, t.unit, "unit" );
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

        auto obj = jv.as_object();
        extract( obj, t.tp, "tp" );
        extract( obj, t.nacc, "nacc" );
        extract( obj, t.values, "values" );
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
            auto obj = tick->as_object();
            extract( obj, t.tick, "tick" );
            extract( obj, t.time, "time" );
            extract( obj, t.nsec, "nsec" );
            if ( auto hv = obj.if_contains( "hv" ) ) {
                extract( hv->as_object(), t.values, "values" );
                if ( auto alarms = hv->as_object().if_contains( "alarms" ) ) {
                    if ( auto alarm = alarms->as_object().if_contains( "alarm" ) ) {
                        extract( alarm->as_object(), t.alarm, "text" );
                    }
                }
            }
            extract( obj, t.adc, "adc" );
        }
    }
    return t;
}

boost_json::boost_json() : jtop_( std::make_unique< boost::json::value >() )
{
}

boost_json::~boost_json()
{
}

bool
boost_json::parse( const std::string& json_string )
{
    boost::system::error_code ec;
    *jtop_ = boost::json::parse( json_string, ec );
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
    return boost::json::serialize( *jtop_ );
}

bool
boost_json::map( data& d )
{
    d = boost::json::value_to< data >( *jtop_ );
    return true;
}

std::string
boost_json::make_json( const data& d )
{
    auto jv = boost::json::value_from( d );
    return boost::json::serialize( jv );
}
