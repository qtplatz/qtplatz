//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request_handler.hpp"
#include "acqiris_protocol.hpp"
#include "reply.hpp"
#include "request.hpp"
#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>

using namespace aqdrv4::server;

request_handler::request_handler()
{
}

void
request_handler::handle_request( boost::asio::streambuf& response
                                 , boost::asio::streambuf& reply )
{
    auto preamble = boost::asio::buffer_cast< const aqdrv4::preamble * >( response.data() );

    std::cout << std::hex << preamble->aug 
              << ", " << preamble->length
              << ", " << preamble->clsid << std::endl;

    response.consume( sizeof( aqdrv4::preamble ) );
    
    std::cout << "consume " << std::dec << sizeof( aqdrv4::preamble )
              << ", remains: " << response.size() << std::endl;

    aqdrv4::preamble ack( aqdrv4::clsid_acknowledge );

    std::ostream request_stream( &reply );
    
    request_stream.write( ack.data(), sizeof( aqdrv4::preamble ) );
    // reply = reply::stock_reply(reply::bad_request);    
}
