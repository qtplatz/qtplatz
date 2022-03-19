//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "tcp_server.hpp"
#include <acqrscontrols/acqiris_waveform.hpp>
#include <acqrscontrols/acqiris_protocol.hpp>
#include <adportable_serializer/portable_binary_oarchive.hpp>
#include <adportable_serializer/portable_binary_iarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <signal.h>
#include <utility>
#include <iostream>

using namespace acqiris::server;

tcp_server::tcp_server(const std::string& address, const std::string& port )
    : io_service_()
    , strand_( io_service_ )
    , signals_( io_service_ )
    , acceptor_( io_service_ )
    , connection_manager_()
    , socket_( io_service_ )
    , request_handler_()
{
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)

    do_await_stop();

    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::tcp::resolver resolver( io_service_ );
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve({address, port});
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    do_accept();
}

void
tcp_server::run()
{
    // The io_service::run() call will block until all asynchronous operations
    // have finished. While the server is running, there is always at least one
    // asynchronous operation outstanding: the asynchronous accept call waiting
    // for new incoming connections.
    io_service_.run();
}

void
tcp_server::stop()
{
    io_service_.stop();
}

void
tcp_server::do_accept()
{
    acceptor_.async_accept(socket_,
                           [this]( boost::system::error_code ec ) {
                               // Check whether the server was stopped by a signal before this
                               // completion handler had a chance to run.
                               if ( !acceptor_.is_open() )  {
                                   return;
                               }

                               if (!ec) {
                                   connection_manager_.start(std::make_shared<connection>(
                                                                 std::move(socket_), connection_manager_, request_handler_));
                               }

                               do_accept();
                           });
}

void
tcp_server::do_await_stop()
{
    signals_.async_wait(
        [this](boost::system::error_code /*ec*/, int /*signo*/)  {

            // The server is stopped by cancelling all outstanding asynchronous
            // operations. Once all operations have finished the io_service::run()
            // call will exit.
            acceptor_.close();
            connection_manager_.stop_all();

        });
}

void
tcp_server::post( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_protocol > p )
{
    connection_manager_.write_all( p );
}

void
tcp_server::post( std::shared_ptr< const acqrscontrols::aqdrv4::waveform > p )
{
    strand_.post( [=,this] {

            if ( auto data = acqrscontrols::aqdrv4::protocol_serializer::serialize( *p ) ) {

                connection_manager_.write_all( data );

            }
        });
}

void
tcp_server::setConnected()
{
}
