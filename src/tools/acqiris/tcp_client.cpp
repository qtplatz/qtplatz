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
#include "tcp_client.hpp"
#include "acqiris_protocol.hpp"
#include <boost/algorithm/string/find.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <istream>
#include <ostream>
#include <string>
#include <iostream>

using namespace aqdrv4::client;

bool tcp_client::debug_mode_ = true;

using boost::asio::ip::tcp;

tcp_client::tcp_client( const std::string& server
                        , const std::string& port )  : resolver_(io_service_)
                                                     , socket_(io_service_)
                                                     , request_( std::make_unique< boost::asio::streambuf >() )
                                                     , status_code_( 0 )
                                                     , error_( NoError )
                                                     , event_stream_( false )
                                                     , server_( server )
{
    std::ostream request_stream( request_.get() );

    aqdrv4::preamble preamble( aqdrv4::clsid_connection_request );    

    request_stream.write( preamble.data(), sizeof( preamble ) );

    tcp::resolver::query query = tcp::resolver::query( server, port );

    resolver_.async_resolve( query
                             , [&]( const boost::system::error_code& err
                                    , tcp::resolver::iterator endpoint_iterator ){
                                 handle_resolve( err, endpoint_iterator );
                             });
}

void
tcp_client::run()
{
    io_service_.run();
}

void
tcp_client::stop()
{
    io_service_.stop();
}

void
tcp_client::setDebug_mode( bool mode )
{
    tcp_client::debug_mode_ = mode;
}

bool
tcp_client::debug_mode()
{
    return tcp_client::debug_mode_;
}

boost::asio::streambuf&
tcp_client::response()
{
    return response_;
}

boost::asio::streambuf&
tcp_client::response_header()
{
    return response_header_;
}

unsigned int
tcp_client::status_code() const
{
    return status_code_;
}

const std::string&
tcp_client::status_message() const
{
    return status_message_;
}

tcp_client::ReplyStatus
tcp_client::error() const
{
    return error_;
}

void
tcp_client::connect( std::function< void( const boost::system::error_code&, boost::asio::streambuf& ) > f )
{
    event_stream_handler_ = f;
}

void
tcp_client::handle_resolve( const boost::system::error_code& err
                           , tcp::resolver::iterator endpoint_iterator)
{
    if ( !err )  {

        tcp::endpoint endpoint = *endpoint_iterator;
        if ( debug_mode_ )
            std::cout << "tcp_client::handle_resolve: " << endpoint << std::endl;

        auto next = ++endpoint_iterator;
        
        socket_.async_connect( endpoint, [=]( const boost::system::error_code& ec ) {
                handle_connect( ec, next );
            });

    } else {

        error_ = Error;
        if ( debug_mode_ )
            std::cout << "[" << server_ << "] Error: " << err.message() << std::endl;
        
    }
}

void
tcp_client::handle_connect(const boost::system::error_code& err,
                       tcp::resolver::iterator endpoint_iterator)
{
    error_code_ = err;

    if (!err)  {
        
        // The connection was successful. Send the request.
        boost::asio::async_write( socket_
                                  , *request_
                                  , [&]( const boost::system::error_code& ec, size_t ) {
                                      handle_write_request( ec );
                                  } );

    } else if ( endpoint_iterator != tcp::resolver::iterator() ) {
        
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
            std::cout << "[" << server_ << "] Error: " << err.message() << std::endl;
    }
}

void
tcp_client::handle_write_request(const boost::system::error_code& err)
{
    error_code_ = err;    

    if ( !err ) {
        // Read the response status line.
        boost::asio::async_read( socket_
                                 , response_
                                 , boost::asio::transfer_at_least( sizeof( aqdrv4::preamble ) )
                                 , [&]( const boost::system::error_code& ec, size_t ) {
                                     std::cout << "reply data" << std::endl;
                                     response_.consume( response_.size() );
                                 });
    } else {
        error_ = Error;
        if ( debug_mode_ )
            std::cout << "[" << server_ << "] Error: " << err.message() << std::endl;
    }
}

