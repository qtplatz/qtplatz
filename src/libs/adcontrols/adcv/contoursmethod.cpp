/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "contoursmethod.hpp"
#include <adportable/debug.hpp>
#include <boost/json.hpp>
#include <boost/exception/all.hpp>
#include <algorithm>
#include <limits>

using namespace adcontrols::adcv;

ContoursMethod::ContoursMethod() : resize_(1)
                                 , blurSize_(0)
                                 , cannyThreshold_{0, 1}
                                 , szThreshold_{ 0, std::numeric_limits< int >::max() }
                                 , kernelSize_( 3 )
                                 , blur_( Blur )

{
}

ContoursMethod::ContoursMethod( const ContoursMethod& t ) : resize_( t.resize_ )
                                                          , blurSize_( t.blurSize_ )
                                                          , cannyThreshold_( t.cannyThreshold_ )
                                                          , szThreshold_( t.szThreshold_ )
                                                          , kernelSize_( t.kernelSize_ )
                                                          , blur_( t.blur_ )
{
}

ContoursMethod::~ContoursMethod()
{
}

void
ContoursMethod::setSizeFactor( int value )
{
    resize_ = std::max( 1, value );
}

void
ContoursMethod::setBlurSize( int value )
{
    blurSize_ = value;
}

void
ContoursMethod::setCannyThreshold( std::pair< int, int >&& value )
{
    cannyThreshold_ = value;
}

void
ContoursMethod::setMinSizeThreshold( unsigned value )
{
    szThreshold_.first = value;
}

void
ContoursMethod::setMaxSizeThreshold( unsigned value )
{
    szThreshold_.second = value;
}

int
ContoursMethod::sizeFactor() const
{
    return std::max( 1, resize_ );
}

int
ContoursMethod::blurSize() const
{
    return blurSize_;
}

std::pair< int, int >
ContoursMethod::cannyThreshold() const
{
    return cannyThreshold_;
}

unsigned
ContoursMethod::minSizeThreshold() const
{
    return szThreshold_.first;
}

unsigned
ContoursMethod::maxSizeThreshold() const
{
    return szThreshold_.second;
}

void
ContoursMethod::setBlur( BlurAlgo algo )
{
    blur_ = algo;
}

ContoursMethod::BlurAlgo
ContoursMethod::blur() const
{
    return blur_;
}

std::string
ContoursMethod::to_json() const
{
    return to_json( *this );
}

std::string
ContoursMethod::to_json( const ContoursMethod& t )
{
    boost::json::value jv = {
        { "ContoursMethod", {
                { "sizeFactor", t.sizeFactor() }
                , { "blurSize",   t.blurSize() }
                , { "cannyThreshold", t.cannyThreshold() }
                , { "minSizeThreshold", t.minSizeThreshold() }
                , { "maxSizeThreshold", t.maxSizeThreshold() }
                , { "blurAlgo", int( t.blur() ) }
            }
        }
    };

    return boost::json::serialize( jv );
}

boost::optional< ContoursMethod >
ContoursMethod::from_json( const std::string& json, boost::system::error_code& ec )
{
    auto jv = boost::json::parse( json, ec );
    if ( ec ) {
        ADDEBUG() << ec;
        return boost::none;
    }

    if ( jv.is_null() )
        return boost::none;

    try {
        if ( auto top = jv.at( "ContoursMethod" ).if_object() ) {
            ContoursMethod t;
            t.setSizeFactor( top->at("sizeFactor").as_int64() );
            t.setBlurSize( top->at( "blurSize" ).as_int64() );
            if ( auto const& a = top->at( "cannyThreshold" ).if_array() ) {
                if ( a->size() == 2 )
                    t.setCannyThreshold( { a->at(0).as_int64(), a->at(1).as_int64() } );
            }
            t.setMinSizeThreshold( top->at( "minSizeThreshold" ).as_int64() );
            t.setMaxSizeThreshold( top->at( "maxSizeThreshold" ).as_int64() );
            t.setBlur( BlurAlgo( top->at("blurAlgo").as_int64() ) );
            return t;
        }
    } catch ( boost::exception& ex ) {
        ADDEBUG() << boost::diagnostic_information_what( ex );
    }
    return boost::none;
}
