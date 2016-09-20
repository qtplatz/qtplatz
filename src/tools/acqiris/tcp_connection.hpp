//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <array>
#include <memory>
#include <boost/asio.hpp>
#include "request_handler.hpp"

namespace acqrscontrols { namespace aqdrv4 {
    class preamble;
    class acqiris_protocol;
}}
    

namespace acqiris {

    namespace server {

        class connection_manager;

        class connection  : public std::enable_shared_from_this<connection>  {
            connection(const connection&) = delete;
            connection& operator=(const connection&) = delete;
        public:
            /// Construct a connection with the given socket.
            explicit connection(boost::asio::ip::tcp::socket socket,
                                connection_manager& manager, request_handler& handler);
            ~connection();

            /// Start the first asynchronous operation for the connection.
            void start();

            /// Stop all asynchronous operations associated with the connection.
            void stop();

            void write( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_protocol > );

        private:
            void do_read();
            void do_write();

            boost::asio::ip::tcp::socket socket_;
            connection_manager& connection_manager_;
            request_handler& request_handler_;

            std::array<char, 8192> buffer_;

            /// The incoming request.
            //request request_;

            /// The parser for the incoming request.
            // request_parser request_parser_;

            boost::asio::streambuf reply_;
            boost::asio::streambuf response_;
            bool connection_requested_;
            bool connected_;
        };

        typedef std::shared_ptr<connection> connection_ptr;

    } // namespace server
} // namespace aqdrv4


