/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC
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
// async_client.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "client.hpp"
#include <adportable/debug.hpp>
#include <istream>
#include <ostream>
#include <string>
#include <boost/algorithm/string/find.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace adurl {

    static boost::asio::ip::tcp::resolver::query make_query( const std::string& server )
    {
        auto sep = server.find( ':' );
        if ( sep != std::string::npos ) {
            return boost::asio::ip::tcp::resolver::query( server.substr( 0, sep ), server.substr( sep + 1 ) );
        } else {
            return boost::asio::ip::tcp::resolver::query( server, "http" );
        }
    }

    static boost::asio::ip::tcp::resolver::query make_query( const std::string& server, const std::string& port )
    {
        auto sep = server.find( ':' );
        if ( sep != std::string::npos ) {
            return boost::asio::ip::tcp::resolver::query( server.substr( 0, sep ), port );
        } else {
            return boost::asio::ip::tcp::resolver::query( server, port );
        }
    }
    
}

using boost::asio::ip::tcp;
using namespace adurl;

bool client::debug_mode_ = false;

client::client( boost::asio::io_service& io_service
                , const std::string& path
                , const std::string& server
                , const std::string& port )  : resolver_(io_service)
                                             , socket_(io_service)
                                             , request_( std::make_unique< boost::asio::streambuf >() )
                                             , response_( std::make_unique< boost::asio::streambuf >() )
                                             , status_code_( 0 )
                                             , error_( NoError )
                                             , event_stream_( false )
                                             , server_( server )
                                               //, port_( port )
{
    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.

    std::ostream request_stream( request_.get() );

    request_stream << "GET " << path << " HTTP/1.0\r\n";
    request_stream << "Host: " << server << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    tcp::resolver::query query = make_query( server, port );
#ifdef NDEBUG
    // due to this makes annoy 'new thread' message on debugger
    resolver_.async_resolve( query
                             , [&]( const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator ){
                                 handle_resolve( err, endpoint_iterator );
                             });
#else
    handle_resolve( boost::system::error_code(), resolver_.resolve( query ) );
#endif
}

client::client(boost::asio::io_service& io_service
               , std::unique_ptr< boost::asio::streambuf >&& request
               , const std::string& server
               , const std::string& port )  : resolver_( io_service )
                                            , socket_( io_service )
                                            , request_( std::move( request ) )
                                            , response_( std::make_unique< boost::asio::streambuf >() )
                                            , status_code_( 0 )
                                            , error_( NoError )
                                            , server_( server )
                                              //, port_( port )
{
    tcp::resolver::query query = make_query( server, port );

    resolver_.async_resolve( query
                             , [&]( const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator ){
                                 handle_resolve( err, endpoint_iterator );
                             });                             
}

void
client::setDebug_mode( bool mode )
{
    client::debug_mode_ = mode;
}

bool
client::debug_mode()
{
    return client::debug_mode_;
}

boost::asio::streambuf&
client::response()
{
    return *response_;
}

boost::asio::streambuf&
client::response_header()
{
    return response_header_;
}

unsigned int
client::status_code() const
{
    return status_code_;
}

const std::string&
client::status_message() const
{
    return status_message_;
}

client::ReplyStatus
client::error() const
{
    return error_;
}

void
client::connect( std::function< void( const boost::system::error_code&, boost::asio::streambuf& ) > f )
{
    event_stream_handler_ = f;
}

void
client::handle_resolve(const boost::system::error_code& err,
                       tcp::resolver::iterator endpoint_iterator )
{
    if ( !err )  {

        tcp::endpoint endpoint = *endpoint_iterator;
        auto next = ++endpoint_iterator;
        
        socket_.async_connect( endpoint, [=]( const boost::system::error_code& ec ) {
                handle_connect( ec, next );
            });

    } else {

        error_ = Error;
        if ( debug_mode_ )
            ADDEBUG() << "[" << server_ << "] Error: " << err.message();

    }
}

void
client::handle_connect(const boost::system::error_code& err,
                       tcp::resolver::iterator endpoint_iterator)
{
    error_code_ = err;

    if (!err)  {

        // The connection was successful. Send the request.
        boost::asio::async_write( socket_
                                  , *request_
                                  , [&]( const boost::system::error_code& ec, size_t ) { handle_write_request( ec ); } );

    } else if (endpoint_iterator != tcp::resolver::iterator()) {
        
        // The connection failed. Try the next endpoint in the list.
        socket_.close();
        tcp::endpoint endpoint = *endpoint_iterator;
        auto next = ++endpoint_iterator;

        socket_.async_connect( endpoint, [=]( const boost::system::error_code& ec ) {
                handle_connect( ec, next );
            });
        
    } else {

        error_ = Error;
        if ( debug_mode_ )
            ADDEBUG() << "[" << server_ << "] Error: " << err.message();
    }
}

void
client::handle_write_request(const boost::system::error_code& err)
{
    error_code_ = err;    

    if (!err) {
        // Read the response status line.
        boost::asio::async_read_until(socket_, *response_, "\r\n"
                                      , [&]( const boost::system::error_code& ec, size_t ) {
                                          handle_read_status_line( ec );
                                      });

    } else {

        error_ = Error;
        if ( debug_mode_ )
            ADDEBUG() << "[" << server_ << "] Error: " << err.message();
    }
}

void
client::handle_read_status_line( const boost::system::error_code& err )
{
    error_code_ = err;
    
    if ( !err )  {

        // Check that response is OK.
        std::istream response_stream( response_.get() );
        response_stream >> http_version_;
        response_stream >> status_code_;
        std::getline( response_stream, status_message_ );
        
        if ( !response_stream || http_version_.substr(0, 5) != "HTTP/")  {
            error_ = Error;
            ADDEBUG() << "Invalid response";
            return;
        }

        if ( status_code_ != 200 )  {
            error_ = Error;
            ADDEBUG() << "[" << server_ << "] Response returned with status code " << status_code_;
            return;
        }
        
        // Read the response headers, which are terminated by a blank line.
        boost::asio::async_read_until( socket_, *response_, "\r\n\r\n"
                                       , [&]( const boost::system::error_code& ec, size_t ) {
                                           handle_read_headers( ec );
                                       });        
    } else  {

        error_ = Error;
        if ( debug_mode_ )
            ADDEBUG() << "[" << server_ << "] Error: " << err.message();
    }
}

void
client::handle_read_headers(const boost::system::error_code& err)
{
    if ( !err )  {
        // Process the response headers.
        std::istream response_stream( response_.get() );
        std::string header;

        std::ostream o( &response_header_ );

        while ( std::getline(response_stream, header) && header != "\r" ) {
            o << header << "\r\n";
            if ( header.find( "Content-Type:" ) != std::string::npos &&
                 header.find( "text/event-stream" ) != std::string::npos ) {
                event_stream_ = true;
            }
            if ( debug_mode_ )
                ADDEBUG() << "[" << server_ << "] " << header;
        }

        if ( event_stream_ ) {
            // Start reading stream
            boost::asio::async_read_until( socket_
                                           , *response_
                                           , "\r\n\r\n"
                                           , [&]( const boost::system::error_code& ec, size_t ) { handle_read_stream( ec ); });
        } else {
            // Start reading remaining data until EOF.
            boost::asio::async_read( socket_
                                     , *response_
                                     , boost::asio::transfer_at_least(1)
                                     , [&]( const boost::system::error_code& ec, size_t ) { handle_read_content( ec ); });
        }
            
    } else {

        error_ = Error;
        if ( debug_mode_ )
            ADDEBUG() << "[" << server_ << "] Error: " << err.message();
    }
}


void
client::handle_read_content(const boost::system::error_code& err)
{
    if ( !err ) {

        // Continue reading remaining data until EOF.
        boost::asio::async_read( socket_
                                 , *response_
                                 , boost::asio::transfer_at_least(1)
                                 , [&]( const boost::system::error_code& ec, size_t ) { handle_read_content( ec ); });
        
    } else if (err != boost::asio::error::eof) {

        error_ = Error;
        if ( debug_mode_ )
            ADDEBUG() << "[" << server_ << "] Error: " << err.message();
    }
}

void
client::handle_read_stream( const boost::system::error_code& ec )
{
    if ( !ec ) {

        if ( event_stream_handler_ ) {

            event_stream_handler_( ec, *response_ );

        } else {
            std::istream response_stream( response_.get() );
            std::string data;
            while ( std::getline( response_stream, data ) && data != "\r" ) {
                if ( debug_mode_ )
                    ADDEBUG() << "[" << server_ << "] DATA: " << data; // consume them
            }
        }

        // Continue reading until empty line
        boost::asio::async_read_until( socket_
                                       , *response_
                                       , "\r\n\r\n"
                                       , [&]( const boost::system::error_code& ec, size_t ) { handle_read_stream( ec ); });
        
    } else if ( ec != boost::asio::error::eof ) {

        error_ = Error;
        if ( debug_mode_ )
            ADDEBUG() << "[" << server_ << "] Error: " << ec;
    }
}
