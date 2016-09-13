//
// connection_manager.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "tcp_connection_manager.hpp"

using namespace aqdrv4::server;

connection_manager::connection_manager()
{
}

void
connection_manager::start(connection_ptr c)
{
    connections_.insert(c);
    c->start();
}

void
connection_manager::stop(connection_ptr c)
{
    connections_.erase(c);
    c->stop();
}

void
connection_manager::stop_all()
{
    for (auto c: connections_)
        c->stop();
    connections_.clear();
}

void
connection_manager::write_all( std::shared_ptr< acqiris_protocol > data )
{
    for (auto c: connections_)
        c->write( data );
}
