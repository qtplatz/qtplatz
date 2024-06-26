//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#if ACQIRIS_DAEMON
#include "mediator.hpp"
#else
#include "document.hpp"
#endif
#include "request_handler.hpp"
#include <acqrscontrols/acqiris_method.hpp>
#include <acqrscontrols/acqiris_protocol.hpp>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>

using namespace acqiris::server;

request_handler::request_handler()
{
}

void
request_handler::handle_request( boost::asio::streambuf& response, boost::asio::streambuf& reply )
{
    using namespace acqrscontrols;
    
    auto preamble = boost::asio::buffer_cast< const aqdrv4::preamble * >( response.data() );
    const char * data = boost::asio::buffer_cast<const char *>( response.data() ) + sizeof( aqdrv4::preamble );

    if ( preamble->clsid == acqrscontrols::aqdrv4::acqiris_method::clsid() ) {

        using acqrscontrols::aqdrv4::protocol_serializer;
        try {
            if ( auto p = protocol_serializer::deserialize< aqdrv4::acqiris_method >( *preamble, data ) ) {

                mediator::instance()->prepare_for_run( p, aqdrv4::allMethod ); // method changed by a sibling client

            }
        } catch ( ... ) {
            ADDEBUG() << boost::current_exception_diagnostic_information();
        }

    } else if ( preamble->clsid == aqdrv4::clsid_event_out ) {

        aqdrv4::pod_reader reader( data, preamble->length );

        mediator::instance()->eventOut( reader.get< uint32_t >() );

    } else if ( preamble->clsid == aqdrv4::clsid_connection_request ) {

        // do noting 

    } else {

        ADDEBUG() << "unhandled protocol: " << aqdrv4::preamble::debug( preamble );

    }

    response.consume( sizeof( aqdrv4::preamble ) + preamble->length );
    
    {
        std::ostream request_stream( &reply );
        acqrscontrols::aqdrv4::preamble ack( acqrscontrols::aqdrv4::clsid_acknowledge );
        request_stream.write( ack.data(), sizeof( acqrscontrols::aqdrv4::preamble ) );
    }
}
