// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "ajax.hpp"
#include "client.hpp"
#include "request.hpp"
#include <string>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
//#include <boost/property_tree/json_parser.hpp>
//#include <boost/property_tree/ptree.hpp>
#include <boost/json.hpp>
#include <adportable/debug.hpp>

namespace adurl {
    std::string server_port_string( const std::string& server, const std::string& port ) {
        if ( ! port.empty() ) {
            if ( port != "80" && port != "http" )
                return server + ":" + port;
        }
        return server;
    }
}

using namespace adurl;

ajax::~ajax()
{
}

ajax::ajax( const std::string& server
            , const std::string& port ) : server_( server )
                                        , port_( port )
{
}


//url := 'http://www.qtplatz.com:8080/foo/bar'
//target := '/dg/ctl$status.json'
//bool
boost::optional< boost::beast::http::response< boost::beast::http::string_body > >
ajax::operator()( const std::string& method
                  , const std::string& target // url
                  , const std::string& mimeType )
{
    namespace http = boost::beast::http;
    const auto verb = boost::beast::http::string_to_verb( method ); //== "GET" ? http::verb::get : http::verb::post;

    http::request< boost::beast::http::empty_body > req{ verb, target, 11 };
    req.set( http::field::host, server_ );
    req.set( http::field::user_agent, BOOST_BEAST_VERSION_STRING );
    req.set( http::field::content_type, mimeType );

    boost::asio::io_service io_service;
    return adurl::client( io_service )( server_, port_, std::move( req ) );
}

boost::optional< boost::beast::http::response< boost::beast::http::string_body > >
ajax::operator()( const std::string& method
                  , const std::string& url
                  , std::string&& body
                  , const std::string& mimeType )
{
    namespace http = boost::beast::http;
    const auto verb = boost::beast::http::string_to_verb( method ); //== "GET" ? http::verb::get : http::verb::post;

    //http::request< boost::beast::http::string_body > req{ verb, url, 11 };
    //req.body() = body;
    boost::beast::http::request<boost::beast::http::string_body> req{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(verb, url, 11)};
    req.set( http::field::host, server_ );
    req.set( http::field::user_agent, BOOST_BEAST_VERSION_STRING );
    req.set( http::field::content_type, mimeType );
    req.prepare_payload();
    boost::asio::io_service io_service;
    return adurl::client( io_service )( server_, port_, std::move( req ) );
}

unsigned int
ajax::status_code() const
{
    return status_code_;
}

const std::string&
ajax::status_message() const
{
    return status_message_;
}

bool
ajax::get_response( boost::json::value& jv ) const
{
    if ( status_code_ == 200 && response_ ) {
        try {
            // std::istream is( response_.get() );
            // boost::property_tree::read_json( is, pt );

            // std::string s( (std::istreambuf_iterator<char>(response_.get())), std::istreambuf_iterator<char>() );
            std::string s( boost::asio::buffer_cast< const char * >(response_->data()), response_->size() );
            jv = boost::json::parse( s );

            return true;
        } catch ( std::exception& ex ) {
        }
     }
    return false;
}

const char *
ajax::get_response( size_t& size ) const
{
    if ( status_code_ == 200 && response_ ) {
        size = response_->size();
        return boost::asio::buffer_cast< const char * >(response_->data());
    }
    size = 0;
    return nullptr;
}

const char *
ajax::response() const
{
    size_t size;
    return get_response( size );
}

std::string
ajax::response_header() const
{
    return response_header_;
}
