//
// connection_manager.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_CONNECTION_MANAGER_HPP
#define HTTP_CONNECTION_MANAGER_HPP


#include "connection.hpp"
#include <mutex>
#include <set>

namespace http {
namespace server {

/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
class connection_manager
{
public:
    connection_manager(const connection_manager&) = delete;
    connection_manager& operator=(const connection_manager&) = delete;

    ~connection_manager();
    connection_manager();

    void start(connection_ptr c);
    void stop(connection_ptr c);
    void stop_all();

    void sse_start( connection_ptr c );
    void sse_stop( connection_ptr c );
    void sse_stop_all();

	void sse_handler( const std::string&, const std::string& id, const std::string& );
    
    inline bool sse_connected() const { return sse_connected_; }
    void sse_connected( bool );

private:
    /// The managed connections.
    std::set<connection_ptr> connections_;
    std::set<connection_ptr> sse_objects_;
    bool sse_connected_;
    std::mutex mutex_;
};

} // namespace server
} // namespace http

#endif // HTTP_CONNECTION_MANAGER_HPP
