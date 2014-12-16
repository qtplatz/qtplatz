/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "document.hpp"
#include <atomic>
#include <mutex>
#include <acewrapper/ifconfig.hpp>
#include <acewrapper/udpeventsender.hpp>
#include <adportable/asio/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using namespace eventbroker;

std::atomic< document * > document::instance_(0);
std::mutex document::mutex_;

document::~document()
{
    io_service_.stop();
    for ( auto& t : threads_ )
        t.join();
}

document::document() : work_( io_service_ )
                     , udpSender_( new acewrapper::udpEventSender( io_service_, "localhost", "7125" ) )
{
    acewrapper::ifconfig::broadaddr( bcast_addrs_ );
    threads_.push_back( adportable::asio::thread( [=] {io_service_.run(); } ) );
    threads_.push_back( adportable::asio::thread( [=] {io_service_.run(); } ) );
}

document *
document::instance()
{
    typedef document T;

    T * tmp = instance_.load( std::memory_order_relaxed );
    std::atomic_thread_fence( std::memory_order_acquire );
    if ( tmp == nullptr ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        tmp = instance_.load( std::memory_order_relaxed );
        if ( tmp == nullptr ) {
            tmp = new T();
            std::atomic_thread_fence( std::memory_order_release );
            instance_.store( tmp, std::memory_order_relaxed );
        }
    }
    return tmp;
}

size_t
document::count_if() const
{
    return bcast_addrs_.size();
}


const char *
document::ifname( size_t idx ) const
{
    if ( idx < count_if() )
        return bcast_addrs_[idx].first.c_str();
    return 0;
}

const char *
document::ifaddr( size_t idx ) const
{
    if ( idx < count_if() )
        return bcast_addrs_[idx].second.c_str();
    return 0;
}

bool
document::register_handler( event_handler h )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    handlers_.push_back( h );
    return true;
}

bool
document::unregister_handler( event_handler h )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    auto it = std::find( handlers_.begin(), handlers_.end(), h );
    if ( it != handlers_.end() )
        handlers_.erase( it );
    return true;
}

void
document::bind( const char * host, const char * port )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    udpSender_.reset( new acewrapper::udpEventSender( io_service_, host, port ) );
}

void
document::event_out( uint32_t value )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( udpSender_ ) {
        std::ostringstream o;
        o << "EVENTOUT " << value << std::endl;
        udpSender_->send_to( o.str() );
    }
}

