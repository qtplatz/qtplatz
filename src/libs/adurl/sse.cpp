// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC
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

#include "sse.hpp"
#include "client.hpp"
#include "request.hpp"
#include <adportable/debug.hpp>
#include <boost/beast/version.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string/trim.hpp>
//#include <boost/utility/string_view.hpp>
#include <algorithm>
#include <cctype>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

using namespace adurl;

namespace {

    class sse_stream {
    public:
        sse_stream() : inserter_( result_ )
                     , device_( inserter_ ) {
        }

        sse_stream& operator << ( std::string&& s ) {
            device_ << std::move( s );
            device_.flush();
            auto pos = result_.find( "\r\n\r\n" );
            if ( pos != std::string::npos ) {
                auto ev = split( boost::string_view( result_ ).substr( 0, pos ) );
                device_.close();
                result_.erase( 0, pos + 4 );
                device_.open( inserter_ );
                ADDEBUG() << "event: " << std::get<0>( ev )
                          << "\tid[" << std::get<1>( ev ) << "]"
                          << "\tdata=" << std::get<2>( ev ).substr( 0, 40 );
            }
            return *this;
        }

        std::tuple< std::string     // event
                    , int32_t       // id
                    , std::string > // data
        split( boost::string_view&& lines ) {
            std::string::size_type pos = 0;
            std::string event, data;
            int32_t id(0);
            do {
                auto crlf = lines.find( "\r\n", pos, 2 );
                auto colon = lines.find( ':', pos );
                auto ident = lines.substr( pos, (colon - pos) );
                auto value = lines.substr( colon + 1, (crlf - colon - 1) );
                if ( ident == "id" ) {
                    id = std::stold( value.to_string(), nullptr );
                } else if ( ident == "event" ) {
                    event = value.to_string();
                    boost::trim( event );
                } else if ( ident == "data" ) {
                    data = value.to_string();
                }
                pos = ( crlf == std::string::npos ) ? std::string::npos : crlf + 2;
            } while ( pos != std::string::npos );

            return std::make_tuple( event, id, data );
        }

        boost::iostreams::back_insert_device< std::string > inserter_;
        boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device_;
        std::string result_;
        std::string frame_;
    };


    namespace http = boost::beast::http;

    template< typename request_type = boost::beast::http::request< boost::beast::http::empty_body > >
    struct sse_functor {
        sse_functor( const sse_functor& t ) = delete;
        const sse_functor& operator = ( const sse_functor& t ) = delete;

        sse_functor( request_type&& req ) : req_( req )
                                          , parser_( res_ ) { // http::response< http::buffer_body >() ) {
            parser_.eager( true );
        }


        void operator()( tcp::socket& socket
                         , boost::system::error_code& errc ) {
            boost::beast::http::async_write(
                socket
                , req_
                , [&]( const boost::system::error_code& ec, size_t ) {
                    do_read_header( socket, ec );
                });
        } // operator

        void do_read_header( tcp::socket& socket, const boost::system::error_code& ec ) {
            if ( !ec ) {
                boost::beast::http::async_read_header(
                    socket
                    , buffer_
                    , parser_
                    , [&]( const boost::system::error_code& ec, size_t ) {
                        if ( !ec )
                            ADDEBUG() << "parser is_done: " << parser_.is_done() << " " << ec;
                        do_read( socket, ec );
                    });
            }
        }

        void do_read( tcp::socket& socket, const boost::system::error_code& ec ) {
            if ( !ec ) {
                parser_.get().body().size = sizeof( sbuf_ );
                parser_.get().body().data = sbuf_;

                boost::beast::http::async_read_some(
                    socket
                    , buffer_
                    , parser_
                    , [&]( const boost::system::error_code& ec, size_t bytes_transfered ) {
                        boost::system::error_code _ec( ec );
                        if ( _ec == boost::beast::http::error::need_buffer )
                            _ec.assign( 0, _ec.category() );
                        sse_stream_ << std::string( sbuf_, sizeof(sbuf_) - parser_.get().body().size );
                        do_read( socket, _ec );
                    });
            }
        }

        request_type req_;
        boost::beast::flat_buffer buffer_{8192}; // (Must persist between reads)
        boost::beast::http::response< boost::beast::http::buffer_body > res_;
        boost::beast::http::response_parser< boost::beast::http::buffer_body > parser_;
        char sbuf_[ 2048 ];
        sse_stream sse_stream_;
    };
}

sse_handler::~sse_handler()
{
}

sse_handler::sse_handler( boost::asio::io_context& ioc ) : ioc_( ioc )
                                                         , client_( std::make_unique< client >( ioc ) )
{
}

bool
sse_handler::connect( const std::string& target
                      , const std::string& host
                      , const std::string& port
                      , sse_event_t handler )
{
    namespace http = boost::beast::http;

    ADDEBUG() << "connect: " << target << ", " << host << ":" << port;

    handler_ = handler;

    http::request< http::empty_body > req{ http::verb::post, target, 11 };
    req.set( http::field::host, host );
    req.set( http::field::user_agent, BOOST_BEAST_VERSION_STRING );
    req.set( http::field::accept, "text/event-stream" );
    req.prepare_payload();
    sse_functor<> fn( std::move( req ) );
    (*client_)( host, port, fn );
    ioc_.run();
}


namespace adurl {
    namespace old {

        class sse::impl {

        public:

            impl( std::unique_ptr< boost::asio::streambuf >&& request
                  , const std::string& server
                  , const std::string& port ) : server_( server )
                                              , work_( io_service_ )
                                              , client_( io_service_, std::move( request ), server_, port ) {

                client_.connect( [&]( const boost::system::error_code& ec
                                      , boost::asio::streambuf& response ){ handle_event( ec, response ); });

            }

            inline static void copy_value( const std::string& data, std::string::size_type pos, std::string& out ) {
                std::move( data.begin() + data.find_first_not_of( " \t", pos )
                           , data.begin() + data.find_first_of( "\r\n", pos ), std::back_inserter( out ) );
            }

            void handle_event( const boost::system::error_code& ec, boost::asio::streambuf& response ) {

                std::pair< std::string, std::string > event_data;

                std::istream response_stream( &response );
                std::string data;

                while ( std::getline( response_stream, data ) && data != "\r" ) {
                    std::string::size_type pos;
                    if ( ( pos = data.find( "event:" ) ) != std::string::npos ) {
                        copy_value( data, pos, event_data.first );
                    } else if ( ( pos = data.find( "data:" ) ) != std::string::npos ) {
                        copy_value( data, pos, event_data.second );
                    }
                }
                callback_( event_data.first.c_str(), event_data.second.c_str() );
            }

        public:
            std::string server_;
            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;
            client client_;
            std::function< void( const char *, const char * ) > callback_;
            std::vector< std::thread > threads_;
        };

        sse::~sse()
        {
            if ( !impl_->threads_.empty() )
                stop();

            delete impl_;
        }


        sse::sse( const char * server, const char * path, const char * port )
        {
            auto request = std::make_unique< boost::asio::streambuf >();
            std::ostream request_stream ( request.get() );

            request_stream << "POST " << request::url_encode( path ) << " HTTP/1.0\r\n";
            request_stream << "Host: " << server << "\r\n";
            request_stream << "Accept: */*\r\n";
            // request_stream << "Connection: close\r\n";
            request_stream << "Content-Type: application/text\r\n";
            request_stream << "\r\n";

            impl_ = new impl( std::move( request ), server, port );
        }

        void
        sse::exec( std::function< void( const char *, const char * ) > callback )
        {
            impl_->callback_ = callback;
            impl_->threads_.emplace_back( [&](){ impl_->io_service_.run(); } );
        }

        void
        sse::stop()
        {
            impl_->io_service_.stop();

            for ( auto& t: impl_->threads_ )
                t.join();

            impl_->threads_.clear();
        }
    } // namespace old
} // namespace adurl
