// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "description.hpp"
#include <ctime>
#include <chrono>
#include <adportable_serializer/portable_binary_oarchive.hpp>
#include <adportable_serializer/portable_binary_iarchive.hpp>
#include <adcontrols/constants.hpp>
#include <adportable/date_time.hpp>
#include <adportable/debug.hpp>
#include <adportable/iso8601.hpp>
#include <adportable/json/extract.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/utf.hpp>
#include <boost/any.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/json.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>

namespace adcontrols {

    template< typename T = description >
    struct description_archive {
        template< typename Archive >
        void operator()( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            if ( version >= 4 ) {
                ar & BOOST_SERIALIZATION_NVP( _.posix_time_ );
                ar & BOOST_SERIALIZATION_NVP( _.keyValue_ );
                ar & BOOST_SERIALIZATION_NVP( _.encode_ );
            } else {
                if ( version < 3 ) {
                    time_t tv_sec;
                    long tv_usec;
                    std::wstring key, value;
                    ar & BOOST_SERIALIZATION_NVP(tv_sec);
                    ar & BOOST_SERIALIZATION_NVP(tv_usec);
                    ar & BOOST_SERIALIZATION_NVP(key);
                    ar & BOOST_SERIALIZATION_NVP(value);
                    std::chrono::system_clock::time_point tp =
                        std::chrono::system_clock::time_point() + std::chrono::seconds( tv_sec ) + std::chrono::microseconds( tv_usec );
                    _.posix_time_ = std::chrono::duration_cast< std::chrono::nanoseconds >( tp.time_since_epoch() ).count();
                    _.keyValue_ = std::make_pair( adportable::utf::to_utf8( key ), adportable::utf::to_utf8( value ) );
                    if ( version < 2 ) {
                        std::wstring wxml;
                        ar & BOOST_SERIALIZATION_NVP( wxml );
                    } else {
                        std::string xml;
                        ar & BOOST_SERIALIZATION_NVP( xml );
                    }
                } else {
                    // V3
                    std::string xml;
                    ar & BOOST_SERIALIZATION_NVP( _.posix_time_ );
                    ar & BOOST_SERIALIZATION_NVP( _.keyValue_ );
                    ar & BOOST_SERIALIZATION_NVP( xml );
                }
            }
        }
    };

    template<> ADCONTROLSSHARED_EXPORT void description::serialize( portable_binary_oarchive& ar, const unsigned int version ) {
        description_archive<>()( ar, *this, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void description::serialize( portable_binary_iarchive& ar, const unsigned int version ) {
        description_archive<>()( ar, *this, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void description::serialize( boost::archive::xml_woarchive& ar, const unsigned int version ) {
        description_archive<>()( ar, *this, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void description::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version ) {
        description_archive<>()( ar, *this, version );
    }
}

namespace {

    struct encode_infer {
        template< typename T >
        adcontrols::TextEncode operator()( const std::basic_string< T >& t ) const {
            ADDEBUG() << "encode_infer: " << t;
            if ( !t.empty() && t.at( 0 ) == '{' ) { // infer json
                boost::system::error_code ec;
                auto jv = boost::json::parse( t, ec );
                if ( !ec )
                    return adcontrols::Encode_JSON;
                ADDEBUG() << jv;
                ADDEBUG() << "----- parse ec=" << ec.message();
            }
            return adcontrols::Encode_TEXT;
        }
    };

}

using namespace adcontrols;

description::~description()
{
}

description::description() : encode_( adcontrols::Encode_TEXT )
{
    posix_time_ = std::chrono::duration_cast< std::chrono::nanoseconds >( std::chrono::system_clock::now().time_since_epoch() ).count();
}

description::description( const std::wstring& key
                          , const std::wstring& text )
    : posix_time_( std::chrono::duration_cast< std::chrono::nanoseconds >( std::chrono::system_clock::now().time_since_epoch() ).count() )
    , keyValue_( std::make_pair( adportable::utf::to_utf8( key ), adportable::utf::to_utf8( text ) ) )
    , encode_( encode_infer()( keyValue_.second ) )
{
}

description::description( std::pair< std::string, std::string >&& keyValue )
    : posix_time_( std::chrono::duration_cast< std::chrono::nanoseconds >( std::chrono::system_clock::now().time_since_epoch() ).count() )
    , keyValue_( std::move( keyValue ) )
    , encode_( encode_infer()( keyValue_.second ) )
{

}

description::description( const description& t ) : posix_time_( t.posix_time_ )
						                         , keyValue_( t.keyValue_ )
                                                 , encode_( t.encode_ )
{
}

std::pair< std::string, std::string >
description::keyValue() const
{
    return keyValue_;
}

void
description::setKey( const std::string& t )
{
    keyValue_.first = t;
}

void
description::setValue( const std::string& t )
{
    keyValue_.second = t;
    encode_ = encode_infer()( t );
}

adcontrols::TextEncode
description::encode() const
{
    return encode_;
}

void
description::setEncode( adcontrols::TextEncode encode )
{
    encode_ = encode;
}

namespace adcontrols {

    template<> ADCONTROLSSHARED_EXPORT std::basic_string< char >
    description::text() const {
        return keyValue_.second;
    }

    template<> ADCONTROLSSHARED_EXPORT std::basic_string< wchar_t >
    description::text() const {
        return adportable::utf::to_wstring( keyValue_.second );
    }

    template<> ADCONTROLSSHARED_EXPORT std::basic_string< char >
    description::key() const {
        return keyValue_.first;
    }

    template<> ADCONTROLSSHARED_EXPORT std::basic_string< wchar_t >
    description::key() const {
        return adportable::utf::to_wstring( keyValue_.first );
    }
}

namespace adcontrols {

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const description& t )
    {
        std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds > tp( std::chrono::nanoseconds( t.posix_time_ ) );
        auto dt = adportable::date_time::to_iso< std::chrono::microseconds >( tp );

        jv = {
            { "posix_time", dt }
            , { "keyValue", boost::json::value_from( t.keyValue_ ) }
            , { "encode",   unsigned( t.encode_ ) }
        };
    }

    description
    tag_invoke( const boost::json::value_to_tag< description >&, const boost::json::value& jv )
    {
        using namespace adportable::json;

        if ( jv.kind() == boost::json::kind::object ) {
            description t;
            auto obj = jv.as_object();
            std::string dt;
            extract( obj, dt, "posix_time" );
            extract( obj, t.keyValue_, "keyValue" );
            extract( obj, reinterpret_cast< unsigned int&>(t.encode_),   "encode" );
            if ( auto tp = adportable::iso8601::parse( dt.begin(), dt.end() ) ) {
                t.posix_time_ = std::chrono::duration_cast< std::chrono::nanoseconds >( tp->time_since_epoch() ).count();
            }
            return t;
        }
        return {};
    }

}
