//
// connection_manager.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection_manager.hpp"
#include "dgctl.hpp"
#include <iostream>

namespace http {
namespace server {

    connection_manager::connection_manager() : sse_connected_( false )
    {
    }

    connection_manager::~connection_manager()
    {
    }

    void
    connection_manager::sse_connected( bool f )
    {
        sse_connected_ = f;
    }

    void
    connection_manager::start(connection_ptr c)
    {
        std::lock_guard< std::mutex > lock( mutex_ );
        connections_.insert(c);
        c->start();
    }

    void
    connection_manager::stop(connection_ptr c)
    {
        std::lock_guard< std::mutex > lock( mutex_ );
        connections_.erase( c );
        c->stop();
    }

    void
    connection_manager::stop_all()
    {
        std::lock_guard< std::mutex > lock( mutex_ );
        for (auto c: connections_)
            c->stop();
        connections_.clear();
    }

    void
    connection_manager::sse_start( connection_ptr c ) 
    {
        if ( !sse_connected_ ) {
			dg::dgctl::instance()->register_sse_handler( [this] ( const std::string& d, const std::string& id, const std::string& ev ){
                    sse_handler( d, id, ev );  } );
            sse_connected_ = true;
        }
        std::lock_guard< std::mutex > lock( mutex_ );
        sse_objects_.insert( c );
        connections_.erase( c );
        c->sse_start();
    }

    void
    connection_manager::sse_stop( connection_ptr c ) 
    {
        std::lock_guard< std::mutex > lock( mutex_ );        
        sse_objects_.erase( c );
        c->stop();
    }

    void
    connection_manager::sse_stop_all()
    {
        std::lock_guard< std::mutex > lock( mutex_ );
        for ( auto c: sse_objects_ )
            c->stop();
        sse_objects_.clear();
    }

    void
	connection_manager::sse_handler( const std::string& data, const std::string& id, const std::string& event )
    {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( ! sse_objects_.empty() ) {
            for ( auto c : sse_objects_ )
				c->sse_write( data, id, event );
        }
    }

} // namespace server
} // namespace http
