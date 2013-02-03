/**************************************************************************
** Copyright (C) 2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

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

void
serialport::async_write( const std::string& data )
{
    async_write( data.c_str(), data.size() );
}

void
serialport::async_write( const char * data, std::size_t length )
{
    if ( ! inbuf_.empty() ) {
        reader_( inbuf_.c_str(), inbuf_.size() );
        inbuf_.clear();
    }
    outbuf_ = std::string( data, length );
    boost::asio::async_write( port_
                              , boost::asio::buffer( &outbuf_[0], outbuf_.size() )
                              , boost::bind( &serialport::handle_write
                                             , this
                                             , boost::asio::placeholders::error
                                             , boost::asio::placeholders::bytes_transferred )
        );
}

void
serialport::async_writeln( const char * data, std::size_t length )
{
    std::string s( data, length );
    s += "\r\n";
    async_write( s.c_str(), length + 2 );
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
        if ( outbuf_.size() == bytes_transferred ) {
            outbuf_.clear();
        } else
            std::cout << bytes_transferred << " octets written but rquired " << outbuf_.size() << std::endl;
    }
}

