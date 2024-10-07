//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP client, asynchronous
//
//------------------------------------------------------------------------------

#include "pug_global.hpp"
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

#include <cstdlib>
#include <future>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

// namespace beast = boost::beast;         // from <boost/beast.hpp>
// namespace http = beast::http;           // from <boost/beast/http.hpp>
// namespace net = boost::asio;            // from <boost/asio.hpp>
// using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Performs an HTTP GET and prints the response

class PUGSHARED_EXPORT session;

class session : public std::enable_shared_from_this<session> {

    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::ssl_stream< boost::beast::tcp_stream > stream_;
    boost::beast::flat_buffer buffer_; // (Must persist between reads)
    boost::beast::http::request< boost::beast::http::empty_body > req_;
    boost::beast::http::response< boost::beast::http::string_body > res_;
    std::promise< boost::beast::http::response< boost::beast::http::string_body > > promise_;
public:
    // Objects are constructed with a strand to
    // ensure that handlers do not execute concurrently.
    // explicit session( boost::asio::io_context& ioc);
    explicit session( boost::asio::any_io_executor ex,  boost::asio::ssl::context& ctx );

    // Start the asynchronous operation
    std::future< boost::beast::http::response< boost::beast::http::string_body > >
    run( const std::string& host, const std::string& port, const std::string& target, int version
         , const std::string& accept = "application/json" );
    void on_resolve( boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results );
    void on_connect( boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type );
    void on_write( boost::beast::error_code ec, std::size_t bytes_transferred );
    void on_read( boost::beast::error_code ec, std::size_t bytes_transferred );
    void on_handshake( boost::beast::error_code ec);
    void on_shutdown( boost::beast::error_code ec);
};

//------------------------------------------------------------------------------
