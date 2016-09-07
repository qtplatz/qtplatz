//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <boost/asio.hpp>

namespace aqdrv4 {
    namespace server {

        struct reply;
        struct request;

        /// The common handler for all incoming requests.
        class request_handler  {
            request_handler(const request_handler&) = delete;
            request_handler& operator=(const request_handler&) = delete;
        public:

            /// Construct with a directory containing files to be served.
            explicit request_handler();

            /// Handle a request and produce a reply.
            void handle_request( boost::asio::streambuf& sbuf, boost::asio::streambuf& rep );

        private:

        };

    } // namespace server
} // namespace aqdrv4


