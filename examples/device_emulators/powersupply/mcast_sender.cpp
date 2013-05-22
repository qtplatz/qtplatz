/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "mcast_sender.hpp"
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include "../hello/lifecycle.hpp"

const short multicast_port = 20001;
const int max_message_count = 999;

char * frame_write( char * d, const std::string& s )
{
    *reinterpret_cast<boost::uint32_t *>(d) = s.size(); d += sizeof(boost::uint32_t);
    BOOST_FOREACH( char c, s )
        *d++ = c;
    return d;
}

char * frame_write( char * d, const char * s, std::size_t octets )
{
    while ( octets-- )
        *d++ = *s++;
    return d;
}

mcast_sender::mcast_sender( boost::asio::io_service& io_service
                            , const boost::asio::ip::address& mcast_address )
    : endpoint_( mcast_address, multicast_port )
    , socket_( io_service, endpoint_.protocol() )
    , timer_( io_service )
    , message_count_(0)
{
    // force sendto data
    handle_timeout( boost::system::error_code() );
}

void
mcast_sender::handle_send_to( const boost::system::error_code& error )
{
    std::cout << "mcast_sender::handle_send_to" << std::endl;
    if ( ! error && message_count_ < max_message_count ) {
        timer_.expires_from_now( boost::posix_time::seconds( 3 ) );
        timer_.async_wait( boost::bind( &mcast_sender::handle_timeout, this
                                        , boost::asio::placeholders::error ) );
    }
}

void
mcast_sender::handle_timeout( const boost::system::error_code& error )
{
    std::cout << "mcast_sender::handle_timeout " << message_count_ << std::endl;
    if ( ! error ) {
        LifeCycleFrame frame( HELO );
        
        boost::array<char, 1500> dbuf;
        char * p = &dbuf[0];
        p = frame_write( p, reinterpret_cast<const char *>( &frame ), sizeof(frame) );
        p = frame_write( p, std::string( "udp" ) );
        p = frame_write( p, std::string( ":7000" ) );     // my well known port#
        p = frame_write( p, std::string( "PS:EI" ) );     // device name
        p = frame_write( p, std::string( "20111205" ) );  // S/N
        p = frame_write( p, std::string( "1.0" ) );       // Rev.
        p = frame_write( p, std::string( "PS2011" ) );    // MODEL
        p = frame_write( p, std::string( "WTI" ) );       // Manufacturer
        std::ostringstream o;
        o << "packet# " << message_count_++;
        p = frame_write( p, o.str() ); // Copyright (debug message)
        
        const std::size_t dlen = p - dbuf.data();
        
        // send local addr(ipaddr/uniq port#) --> remote addr( multicast/20001 )
        // if no ipaddr determined, controller will assign ipaddr by using mcast/bcast

        socket_.async_send_to( boost::asio::buffer( dbuf.data(), dlen )
                               , endpoint_
                               , boost::bind( &mcast_sender::handle_send_to, this
                                              , boost::asio::placeholders::error ) );        
    }
}
