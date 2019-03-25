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
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>

namespace adcontrols {
    using namespace boost::serialization;

    template< typename Archive > ADCONTROLSSHARED_EXPORT void description::serialize(Archive& ar, const unsigned int version ) {
        using namespace boost::serialization;
        if ( version < 3 ) {
            time_t tv_sec;
            long tv_usec;
            std::wstring key, value;
            ar & BOOST_SERIALIZATION_NVP(tv_sec);
            ar & BOOST_SERIALIZATION_NVP(tv_usec);
            ar & BOOST_SERIALIZATION_NVP(key);
            ar & BOOST_SERIALIZATION_NVP(value);
            std::chrono::system_clock::time_point tp = std::chrono::system_clock::time_point() + std::chrono::seconds( tv_sec ) + std::chrono::microseconds( tv_usec );
            posix_time_ = std::chrono::duration_cast< std::chrono::nanoseconds >( tp.time_since_epoch() ).count();
            keyValue_ = std::make_pair( adportable::utf::to_utf8( key ), adportable::utf::to_utf8( value ) );
            if ( version < 2 ) {
                std::wstring wxml;
                ar & BOOST_SERIALIZATION_NVP( wxml );
                xml_ = adportable::utf::to_utf8( wxml );
            } else {
                ar & BOOST_SERIALIZATION_NVP( xml_ );
            }
        } else {
            ar & BOOST_SERIALIZATION_NVP( posix_time_ );
            ar & BOOST_SERIALIZATION_NVP( keyValue_ );
            ar & BOOST_SERIALIZATION_NVP( xml_ );
        }
    }

    template<> ADCONTROLSSHARED_EXPORT void description::serialize( portable_binary_oarchive& ar, const unsigned int version ) {
        description::serialize( ar, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void description::serialize( portable_binary_iarchive& ar, const unsigned int version ) {
        description::serialize( ar, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void description::serialize( boost::archive::xml_woarchive& ar, const unsigned int version ) {
        description::serialize( ar, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void description::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version ) {
        description::serialize( ar, version );
    }
}

using namespace adcontrols;

description::~description()
{
}

description::description()
{
    posix_time_ = std::chrono::duration_cast< std::chrono::nanoseconds >( std::chrono::system_clock::now().time_since_epoch() ).count();
}

description::description( const wchar_t * key, const wchar_t * text )
{
    posix_time_ = std::chrono::duration_cast< std::chrono::nanoseconds >( std::chrono::system_clock::now().time_since_epoch() ).count();
    keyValue_ = std::make_pair( adportable::utf::to_utf8( key ), adportable::utf::to_utf8( text ) );
}

description::description( const std::wstring& key, const std::wstring& text )
{
    posix_time_ = std::chrono::duration_cast< std::chrono::nanoseconds >( std::chrono::system_clock::now().time_since_epoch() ).count();
    keyValue_ = std::make_pair( adportable::utf::to_utf8( key ), adportable::utf::to_utf8( text ) );
}

description::description( std::pair< std::string, std::string >&& keyValue ) : keyValue_( keyValue )
{
}

description::description( const description& t ) : posix_time_( t.posix_time_ )
						                         , keyValue_( t.keyValue_ )
                                                 , xml_( t.xml_ )
{
}

std::pair< std::string, std::string >
description::keyValue() const
{
    return keyValue_;
}

std::wstring
description::text() const
{
    return adportable::utf::to_wstring( keyValue_.second );
}

std::wstring
description::key() const
{
    return adportable::utf::to_wstring( keyValue_.first );
}

const char *
description::xml() const
{
    return xml_.c_str();
}

void
description::xml( const char * u )
{
    xml_ = u;
}
