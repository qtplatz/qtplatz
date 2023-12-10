//
// Copyright ( c ) 2016-2019 Vinnie Falco ( vinnie dot falco at gmail dot com )
//
// Distributed under the Boost Software License, Version 1.0. ( See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt )
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP client, asynchronous
//
//------------------------------------------------------------------------------

#include "http_client_async.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
namespace {
    void
    fail( beast::error_code ec, char const* what )
    {
        std::cerr << what << ": " << ec.message() << "\n";
    }
}


session::session(  boost::asio::any_io_executor ex,  boost::asio::ssl::context& ctx  )
    : resolver_( ex )
    , stream_( ex, ctx )
{
}

// Start the asynchronous operation
std::future< boost::beast::http::response< boost::beast::http::string_body > >
session::run(  char const* host, char const* port, char const* target, int version )
{
    // Set SNI Hostname ( many hosts need this to handshake successfully )
    if ( ! SSL_set_tlsext_host_name( stream_.native_handle(), host )  )    {
        beast::error_code ec{static_cast<int>( ::ERR_get_error() ), net::error::get_ssl_category()};
        std::cerr << ec.message() << "\n";
        return {};
    }

    // Set up an HTTP GET request message
    req_.version(  version  );
    req_.method(  http::verb::get );
    req_.target(  target  );
    req_.set(  http::field::host, host );
    req_.set(  http::field::user_agent, BOOST_BEAST_VERSION_STRING );
    req_.set(  http::field::accept, "application/json" ); //"chemical/x-mdl-sdfile" );

    // Look up the domain name
    resolver_.async_resolve(   host, port
                              , beast::bind_front_handler(  &session::on_resolve
                                                           , shared_from_this() ) );
    return promise_.get_future();
}

void
session::on_resolve(  beast::error_code ec,  tcp::resolver::results_type results  )
{
    if (  ec  )
        return fail( ec, "resolve" );

    // Set a timeout on the operation
    beast::get_lowest_layer( stream_ ).expires_after( std::chrono::seconds( 30 ) );

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer( stream_ ).async_connect(   results
                                                     , beast::bind_front_handler(
                                                         &session::on_connect
                                                         , shared_from_this() ) );
}

void
session::on_connect( beast::error_code ec, tcp::resolver::results_type::endpoint_type )
{
    if (  ec  )
        return fail( ec, "connect" );

    // Perform the SSL handshake
    stream_.async_handshake(  boost::asio::ssl::stream_base::client
                             ,  beast::bind_front_handler(  &session::on_handshake
                                                           , shared_from_this()  )
         );
}

void
session::on_handshake( beast::error_code ec )
{
    if (  ec  )
        return fail( ec, "handshake" );

    // Set a timeout on the operation
    beast::get_lowest_layer( stream_ ).expires_after( std::chrono::seconds( 30 ) );

    // Send the HTTP request to the remote host
    http::async_write( stream_, req_
                      , beast::bind_front_handler(  &session::on_write
                                                   , shared_from_this() ) );
}

void
session::on_write(  beast::error_code ec, std::size_t bytes_transferred  )
{
    boost::ignore_unused( bytes_transferred );

    if (  ec  )
        return fail( ec, "write" );

    // Receive the HTTP response
    http::async_read( stream_, buffer_, res_,
                     beast::bind_front_handler( &session::on_read
                                                , shared_from_this() ) );
}

void
session::on_read(  beast::error_code ec, std::size_t bytes_transferred )
{
    boost::ignore_unused( bytes_transferred );

    if (  ec  )
        return fail( ec, "read" );

    // Write the message to standard out
    promise_.set_value( res_ );

    // Set a timeout on the operation
    beast::get_lowest_layer( stream_ ).expires_after( std::chrono::seconds( 30 ) );

    // Gracefully close the stream
    stream_.async_shutdown(
        beast::bind_front_handler(   &session::on_shutdown
                                     , shared_from_this() ) );
}

void
session::on_shutdown( beast::error_code ec )
{
    if(  ec == net::error::eof  )  {
        // Rationale:
        // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
        ec = {};
    }
    if (  ec  )
        return fail( ec, "shutdown" );

    // If we get here then the connection is closed gracefully
}
