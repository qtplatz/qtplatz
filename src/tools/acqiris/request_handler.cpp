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
    auto preamble = boost::asio::buffer_cast< const acqrscontrols::aqdrv4::preamble * >( response.data() );
    const char * data = boost::asio::buffer_cast<const char *>( response.data() ) + sizeof( preamble );

    ADDEBUG() << "*** request_handler: " << acqrscontrols::aqdrv4::preamble::debug( preamble );

    if ( preamble->clsid == acqrscontrols::aqdrv4::acqiris_method::clsid() ) {

        using acqrscontrols::aqdrv4::protocol_serializer;
        
        if ( auto p = protocol_serializer::deserialize< acqrscontrols::aqdrv4::acqiris_method >( *preamble, data ) ) {
            document::instance()->handleValueChanged( p, acqrscontrols::aqdrv4::allMethod );
            document::instance()->acqiris_method_adapted( p );
        }
    }

    response.consume( sizeof( preamble ) + preamble->length );
    
    std::cout << "consume " << std::dec << sizeof( preamble )
              << ", remains: " << response.size() << std::endl;

    acqrscontrols::aqdrv4::preamble ack( acqrscontrols::aqdrv4::clsid_acknowledge );

    std::ostream request_stream( &reply );
    
    request_stream.write( ack.data(), sizeof( preamble ) );
}
