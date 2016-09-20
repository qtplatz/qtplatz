/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC
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

#include "document.hpp"
#include "tcp_client.hpp"
#include "tcp_task.hpp"
#include <acqrscontrols/acqiris_waveform.hpp>
#include <acqrscontrols/acqiris_protocol.hpp>
#include <acqrscontrols/acqiris_method.hpp>
#include <adportable/debug.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <istream>
#include <ostream>
#include <string>
#include <iostream>

using namespace acqiris::client;

bool tcp_client::debug_mode_ = true;

using boost::asio::ip::tcp;

tcp_client::tcp_client( const std::string& server
                        , const std::string& port )  : resolver_(io_service_)
                                                     , socket_(io_service_)
                                                     , request_( std::make_unique< boost::asio::streambuf >() )
                                                     , status_code_( 0 )
                                                     , error_( NoError )
                                                     , event_stream_( false )
                                                     , server_( server )
{
    std::ostream request_stream( request_.get() );

    acqrscontrols::aqdrv4::preamble preamble( acqrscontrols::aqdrv4::clsid_connection_request );    
    
    request_stream.write( preamble.data(), sizeof( preamble ) );
    
    tcp::resolver::query query = tcp::resolver::query( server, port );

    resolver_.async_resolve( query
                             , [&]( const boost::system::error_code& err
                                    , tcp::resolver::iterator endpoint_iterator ){
                                 handle_resolve( err, endpoint_iterator );
                             });
}

void
tcp_client::run()
{
    io_service_.run();
}

void
tcp_client::stop()
{
    io_service_.stop();
}

void
tcp_client::setDebug_mode( bool mode )
{
    tcp_client::debug_mode_ = mode;
}

bool
tcp_client::debug_mode()
{
    return tcp_client::debug_mode_;
}

boost::asio::streambuf&
tcp_client::response()
{
    return response_;
}

boost::asio::streambuf&
tcp_client::response_header()
{
    return response_header_;
}

unsigned int
tcp_client::status_code() const
{
    return status_code_;
}

const std::string&
tcp_client::status_message() const
{
    return status_message_;
}

tcp_client::ReplyStatus
tcp_client::error() const
{
    return error_;
}

void
tcp_client::connect( std::function< void( const boost::system::error_code&, boost::asio::streambuf& ) > f )
{
    event_stream_handler_ = f;
}

void
tcp_client::handle_resolve( const boost::system::error_code& err
                            , tcp::resolver::iterator endpoint_iterator)
{
    if ( !err )  {

        tcp::endpoint endpoint = *endpoint_iterator;
        if ( debug_mode_ )
            std::cout << "tcp_client::handle_resolve: " << endpoint << std::endl;

        auto next = ++endpoint_iterator;
        
        socket_.async_connect( endpoint, [=]( const boost::system::error_code& ec ) {
                handle_connect( ec, next );
            });

    } else {

        error_ = Error;
        if ( debug_mode_ )
            std::cout << "[" << server_ << "] Error: " << err.message() << std::endl;
        
    }
}

void
tcp_client::handle_connect( const boost::system::error_code& err,
                            tcp::resolver::iterator endpoint_iterator )
{
    error_code_ = err;

    if (!err)  {

        ADDEBUG() << "connection successed: " << socket_.remote_endpoint() << " - " << socket_.local_endpoint();
        
        // The connection was successful. Send the request.
        boost::asio::async_write( socket_
                                  , *request_
                                  , [&]( const boost::system::error_code& ec, size_t ) {
                                      handle_write_request( ec );
                                  } );

        tcp_task::instance()->initialize();

    } else if ( endpoint_iterator != tcp::resolver::iterator() ) {
        
        // The connection failed. Try the next endpoint in the list.
        socket_.close();
        
        tcp::endpoint endpoint = *endpoint_iterator;
        auto next = ++endpoint_iterator;

        socket_.async_connect( endpoint, [=]( const boost::system::error_code& ec ) {
                handle_connect( ec, next );
            });
        
    } else {

        error_ = Error;
        if ( debug_mode_ )
            std::cout << "[" << server_ << "] Error: " << err.message() << std::endl;
    }
}

void
tcp_client::handle_write_request(const boost::system::error_code& err)
{
    error_code_ = err;

    if ( !err ) {

        // Read the response status line.
        boost::asio::async_read(
            socket_
            , response_
            , boost::asio::transfer_at_least( sizeof( acqrscontrols::aqdrv4::preamble ) )
            , [&]( const boost::system::error_code& ec, size_t ) {
                
                auto preamble = boost::asio::buffer_cast< const acqrscontrols::aqdrv4::preamble * >( response_.data() );

                acqrscontrols::aqdrv4::preamble::dump( preamble, response_.size() );
                
                if ( acqrscontrols::aqdrv4::preamble::isOk( preamble ) && 
                     preamble->length <= response_.size() - sizeof( acqrscontrols::aqdrv4::preamble ) ) {
                    
                    if ( preamble->clsid == acqrscontrols::aqdrv4::clsid_acknowledge ) {
                        ADDEBUG() << ">>> tcp_client: got ACK\t" << acqrscontrols::aqdrv4::preamble::debug( preamble );
                        response_.consume( sizeof( acqrscontrols::aqdrv4::preamble ) + preamble->length );
                        do_read();
                        
                    } else {
                        ADDEBUG() << ">>> tcp_client: got UNKNOWN\t" << acqrscontrols::aqdrv4::preamble::debug( preamble );
                        response_.consume( response_.size() );
                        do_read();
                        
                    }
                }
            });

    } else {
        
        error_ = Error;
        if ( debug_mode_ )
            ADDEBUG() << "[" << server_ << "] Error: " << err.message();
    }
}

void
tcp_client::do_read()
{
    using acqrscontrols::aqdrv4::protocol_serializer;
    using acqrscontrols::aqdrv4::waveform;
    
    boost::asio::async_read(
        socket_
        , response_
        , boost::asio::transfer_at_least( 1 )
        , [&]( const boost::system::error_code& ec, size_t ) {
            
            if ( !ec ) {
                
                auto preamble = boost::asio::buffer_cast< const acqrscontrols::aqdrv4::preamble * >( response_.data() );
                
                if ( acqrscontrols::aqdrv4::preamble::isOk( preamble ) && 
                     preamble->length <= response_.size() - sizeof( acqrscontrols::aqdrv4::preamble ) ) {
                    
                    const char * data = boost::asio::buffer_cast<const char *>( response_.data() ) + sizeof( acqrscontrols::aqdrv4::preamble );

                    if ( preamble->clsid == waveform::clsid() ) {

                        if ( auto p = protocol_serializer::deserialize<waveform>( *preamble, data ) )
                            tcp_task::instance()->push( p );
                        
                    } else if ( preamble->clsid == acqrscontrols::aqdrv4::acqiris_method::clsid() ) {

                        if ( auto p = protocol_serializer::deserialize< acqrscontrols::aqdrv4::acqiris_method >( *preamble, data ) )
                            tcp_task::instance()->push( p );
                        
                    } else if ( preamble->clsid == acqrscontrols::aqdrv4::clsid_temperature ) {

                        acqrscontrols::aqdrv4::pod_reader reader( data, preamble->length );
                        document::instance()->replyTemperature( reader.get< int32_t >() );
                    } else {
                        ADDEBUG() << "Error: " << acqrscontrols::aqdrv4::preamble::debug( preamble );
                    }

                    response_.consume( sizeof( acqrscontrols::aqdrv4::preamble ) + preamble->length );
                    
                } else {
                    // ADDEBUG() << "Reading " << preamble->length << "\t" << acqrscontrols::aqdrv4::preamble::debug( preamble );
                }

                do_read();                                         

            } else {
                ADDEBUG() << __FILE__ << ":" << __LINE__ << ">>> do_read: failed: " << ec.message();

                socket_.close();
                document::instance()->close_client();
            }

        });
}

void
tcp_client::write( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_protocol > data )
{
    auto self( this );

    using namespace acqrscontrols;

    boost::asio::async_write(
        socket_
        , data->to_buffers()
        , [this, self, data]( boost::system::error_code ec, std::size_t ) {
            if ( ec ) {
                ADDEBUG() << "*** error tcp_client::write: " << ec.message();
            } else
                ADDEBUG() << "data wrote: " << aqdrv4::preamble::debug( &data->preamble() );
        });
}
