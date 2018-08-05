// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC
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
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <adportable/debug.hpp>

#if 0 // todo
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/qi.hpp>

namespace adurl {

    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;
    
    struct url {
        std::string protocol;
        std::string address;
        std::string port;
        std::string path;
        url() {}
        url( const url& t ) : protocol( t.protocol )
                            , address( t.address )
                            , port( t.port )
                            , path( t.path ) {
        }
    };
}

// this has to be a global scope
BOOST_FUSION_ADAPT_STRUCT(
    adurl::url,
    (std::string, protocol)
    (std::string, address)
    (std::string, port)
    (std::string, path)
    )

namespace adurl {

    template< typename Iterator >
    struct url_parser : boost::spirit::qi::grammer< Iterator, url() > {
        
        url_parser() : url_parser::base_type( start ) {
            using qi::lit;
            using qi::lexeme;

            //string %= (+(char_))
            string = ( +( char_ ) );

            start %= string_
                >> lit("://")
                >> string_ >> (":") >> string_
        }

        qi::rule< Iterator, std::string() > string; //protocol, address, port, path;
        boost::spirit::qi::rule< Iterator, url_type() > start;
    };
}
#endif

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


// url := 'http://www.qtplatz.com:8080/foo/bar'
bool
ajax::operator()( const std::string& method
                  , const std::string& url
                  , const std::string& mimeType )
{
    auto request = std::make_unique< boost::asio::streambuf >();
    std::ostream request_stream ( request.get() );

    request_stream << method << " " << request::url_encode( url ) << " HTTP/1.0\r\n";
    request_stream << "Host: " << server_port_string( server_, port_ ) << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n";
    request_stream << "Content-Type: " << mimeType << "\r\n";  //"Content-Type: application/json\r\n";
    request_stream << "\r\n";

    boost::asio::io_service io_service;
    adurl::client client( io_service, std::move( request ), server_, port_ );

    io_service.run();

    status_code_ = client.status_code();
    status_message_ = std::move( client.status_message_ );

    response_header_ = std::string( (std::istreambuf_iterator<char>(&client.response_header())), std::istreambuf_iterator<char>() );

    if ( status_code_ == 200 )
        response_ = std::move( client.response_ );

    if ( adurl::client::debug_mode() ) {
        ADDEBUG() << "\n-----------------------------------"
                  << "\n" << &client.response_header()
                  << "\nstatus_code: " << status_code_ << ", " << status_message_
                  << "\n-----------------------------------";
    }

    return client.error() == adurl::client::NoError && client.status_code() == 200;
}

bool
ajax::operator()( const std::string& method
                  , const std::string& url
                  , const std::string& body 
                  , const std::string& mimeType )
{
    auto request = std::make_unique< boost::asio::streambuf >();
    std::ostream request_stream ( request.get() );

    request_stream << method << " " << request::url_encode( url ) << " HTTP/1.0\r\n";
    request_stream << "Host: " << server_port_string( server_, port_ ) << "\r\n";    
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n";
    request_stream << "Content-Type: " << mimeType << "\r\n";  //"Content-Type: application/json\r\n";
    request_stream << "Content-Length: " << body.size() << "\r\n";
    request_stream << "\r\n";
    request_stream << body << "\r\n";

    boost::asio::io_service io_service;
    adurl::client client( io_service, std::move( request ), server_, port_ );

    io_service.run();

    status_code_ = client.status_code();
    status_message_ = std::move( client.status_message_ );

    response_header_ = std::string( (std::istreambuf_iterator<char>(&client.response_header())), std::istreambuf_iterator<char>() );

    if ( status_code_ == 200 )
        response_ = std::move( client.response_ );

    if ( adurl::client::debug_mode() ) {
        ADDEBUG() << "\n-----------------------------------"
                  << "\n" << &client.response_header()
                  << "\nstatus_code: " << status_code_ << ", " << status_message_
                  << "\n-----------------------------------";
    }
    return client.error() == adurl::client::NoError && client.status_code() == 200;
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
ajax::get_response( boost::property_tree::ptree& pt ) const
{
    if ( status_code_ == 200 && response_ ) {
        try {
            std::istream is( response_.get() );
            boost::property_tree::read_json( is, pt );
            return true;
        } catch ( boost::property_tree::json_parser::json_parser_error& ) {            
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
