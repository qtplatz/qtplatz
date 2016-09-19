//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <acqrscontrols/acqiris_method.hpp>
#include "acqiris_protocol.hpp"
#include "document.hpp"
#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
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
    const char * data = boost::asio::buffer_cast<const char *>( response.data() ) + sizeof( aqdrv4::preamble );

    ADDEBUG() << "*** request_handler: " << aqdrv4::preamble::debug( preamble );

    if ( preamble->clsid == aqdrv4::acqiris_method::clsid() ) {
        if ( auto p = protocol_serializer::deserialize< aqdrv4::acqiris_method >( *preamble, data ) ) {
            document::instance()->handleValueChanged( p, aqdrv4::allMethod );
            document::instance()->acqiris_method_adapted( p );
        }
    }

    response.consume( sizeof( aqdrv4::preamble ) + preamble->length );
    
    std::cout << "consume " << std::dec << sizeof( aqdrv4::preamble )
              << ", remains: " << response.size() << std::endl;

    aqdrv4::preamble ack( aqdrv4::clsid_acknowledge );

    std::ostream request_stream( &reply );
    
    request_stream.write( ack.data(), sizeof( aqdrv4::preamble ) );
}
