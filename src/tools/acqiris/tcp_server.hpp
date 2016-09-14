//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <boost/asio.hpp>
#include <string>
#include "tcp_connection.hpp"
#include "tcp_connection_manager.hpp"
#include "request_handler.hpp"

class waveform;

namespace aqdrv4 {
    namespace server {

        /// The top-level class of the HTTP server.
        class tcp_server  {
            tcp_server(const tcp_server&) = delete;
            tcp_server& operator=(const tcp_server&) = delete;
        public:
            
            /// Construct the server to listen on the specified TCP address and port, and
            /// serve up files from the given directory.
            explicit tcp_server( const std::string& address, const std::string& port );

            /// Run the server's io_service loop.
            void run();
            void stop();
            void setConnected();
            void post( std::shared_ptr< acqiris_protocol > );
            
        private:
            /// Perform an asynchronous accept operation.
            void do_accept();

            /// Wait for a request to stop the server.
            void do_await_stop();
            
            boost::asio::io_service io_service_;
            boost::asio::io_service::strand strand_;

            /// The signal_set is used to register for process termination notifications.
            boost::asio::signal_set signals_;

            /// Acceptor used to listen for incoming connections.
            boost::asio::ip::tcp::acceptor acceptor_;

            /// The connection manager which owns all live connections.
            connection_manager connection_manager_;

            /// The next socket to be accepted.
            boost::asio::ip::tcp::socket socket_;

            /// The handler for all incoming requests.
            request_handler request_handler_;

            bool hasClient_;
        };

    } // namespace server
} // namespace aqdrv4


