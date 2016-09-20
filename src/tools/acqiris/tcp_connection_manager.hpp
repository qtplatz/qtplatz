//
// connection_manager.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <set>
#include "tcp_connection.hpp"

namespace acqrscontrols {
    namespace aqdrv4 {
        class preamble;
        class acqiris_protocol;
    }
}

namespace acqiris {

    namespace server {

        /// Manages open connections so that they may be cleanly stopped when the server
        /// needs to shut down.

        class connection_manager {
            connection_manager(const connection_manager&) = delete;
            connection_manager& operator=(const connection_manager&) = delete;
        public:

            /// Construct a connection manager.
            connection_manager();

            /// Add the specified connection to the manager and start it.
            void start(connection_ptr c);

            /// Stop the specified connection.
            void stop(connection_ptr c);

            /// Stop all connections.
            void stop_all();

            void write_all( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_protocol > );

        private:
            /// The managed connections.
            std::set<connection_ptr> connections_;
        };

    } // namespace server
} // namespace aqdrv4


