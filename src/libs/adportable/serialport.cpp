/**************************************************************************
** Copyright (C) 2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include "serialport.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <compiler/diagnostic_pop.h>

#include "debug.hpp"

using namespace adportable;

serialport::serialport( boost::asio::io_service& io_service
                        , const char * device_name
                        , unsigned int baud_rate ) : port_( io_service )
{
    if ( device_name )
        open( device_name, baud_rate );
}

void
serialport::close()
{
    port_.close();
}

bool
serialport::open( const std::string& device_name, unsigned int baud_rate )
{
    port_.open( device_name );

    if ( ! port_.is_open() )
        return false;

    if ( baud_rate > 0 ) {
        typedef boost::asio::serial_port_base asio_serial;
        
        port_.set_option( asio_serial::baud_rate( baud_rate ) );

        // application can override options using 'operator boost::asio::serial_port& ()'
        port_.set_option( asio_serial::flow_control( asio_serial::flow_control::none ) );
        port_.set_option( asio_serial::parity( asio_serial::parity::none ) );
        port_.set_option( asio_serial::stop_bits( asio_serial::stop_bits::one ) );
        port_.set_option( asio_serial::character_size( 8 ) );

    }
    return true;
}

void
serialport::write( const char * data, std::size_t length )
{
     boost::asio::write( port_, boost::asio::buffer( data, length ) );
}

bool
serialport::write( const char * data, std::size_t length, unsigned long milliseconds )
{
#if defined BOOST_THREAD
    boost::mutex::scoped_lock lock( mutex_ );
#else
    std::lock_guard< std::mutex > lock( mutex_ );
#endif
    outbuf_ = std::string( data, length );
    boost::asio::async_write( port_
                              , boost::asio::buffer( &outbuf_[0], outbuf_.size() )
                              , boost::bind( &serialport::handle_write
                                             , this
                                             , boost::asio::placeholders::error
                                             , boost::asio::placeholders::bytes_transferred )
        );
    boost::system_time timeout( boost::get_system_time() + boost::posix_time::milliseconds( milliseconds ) );
    return cond_.timed_wait( lock, timeout );
}

void
serialport::start()
{
    initiate_read();
}

void
serialport::async_reader( boost::function< void ( const char *, std::size_t ) > reader )
{
    reader_ = reader;
}

void
serialport::initiate_read()
{
    read_buffer_.consume( read_buffer_.size() );
    port_.async_read_some( boost::asio::buffer( read_buffer_.prepare( 256 ) )
                           , boost::bind( &serialport::handle_read
                                          , this
                                          , boost::asio::placeholders::error
                                          , boost::asio::placeholders::bytes_transferred )
        );
}

void
serialport::handle_read( const boost::system::error_code& error, std::size_t bytes_transferred )
{
    if ( !error ) {
        read_buffer_.commit( bytes_transferred );
        std::istream is( &read_buffer_ );
        while ( is.good() ) {
            char c;
            if ( is.get( c ).good() ) {
                inbuf_ += c;
                if ( c == '\r' || c == '\n' ) {
                    reader_( inbuf_.c_str(), inbuf_.size() );
                    inbuf_.clear();
                }
            }
        }
    }
    initiate_read();
}

void
serialport::handle_write( const boost::system::error_code& error, std::size_t bytes_transferred )
{
    if ( !error ) {
        boost::mutex::scoped_lock lock( mutex_ );
        if ( outbuf_.size() == bytes_transferred ) {
            outbuf_.clear();
            cond_.notify_one();        
        } else
            adportable::debug(__FILE__, __LINE__ ) 
                << bytes_transferred << " octets written but rquired " << outbuf_.size();
        
    } else 
        adportable::debug(__FILE__, __LINE__) << "handle_write: " << error;
}

