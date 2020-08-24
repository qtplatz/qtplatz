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

    // typedef std::tuple< std::string, int32_t, std::string > sse_event_type;

    class sse_stream {
    public:
        sse_stream() : inserter_( result_ )
                     , device_( inserter_ ) {
        }

        boost::optional< sse_event_data_t > operator << ( std::string&& s ) {
            device_ << std::move( s );
            device_.flush();
            auto pos = result_.find( "\r\n\r\n" );
            if ( pos != std::string::npos ) {
                auto ev = split( boost::string_view( result_ ).substr( 0, pos ) );
                device_.close();
                result_.erase( 0, pos + 4 );
                device_.open( inserter_ );
                return std::move( ev );
            }
            return boost::none;
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


    ////////////  boost::asio /////////////
    template< typename request_type = boost::beast::http::request< boost::beast::http::empty_body > >
    struct sse_functor_a {
        sse_functor_a( const sse_functor_a& t ) = delete;
        const sse_functor_a& operator = ( const sse_functor_a& t ) = delete;

        sse_functor_a( request_type&& req
                       , std::function< void( sse_event_data_t&& ) > handler )
            : req_( req )
            , handler_( handler )
            , sse_response_( std::make_unique< boost::asio::streambuf >() ) {

        }

        //////////////////
        void operator()( tcp::socket& socket
                         , boost::system::error_code& errc ) {
            boost::beast::http::async_write(
                socket
                , req_
                , [&]( const boost::system::error_code& ec, size_t ) {
                    //do_read_header( socket, ec );
                    do_asio_read_header( socket, ec );
                });
        } // operator

    private:
        ////// asio impl //////////
        void do_asio_read_header( tcp::socket& socket, const boost::system::error_code& ec ) {
            if ( !ec ) {
                boost::asio::async_read_until(
                    socket
                    , *sse_response_
                    , "\r\n\r\n"
                    , [&]( const boost::system::error_code& ec, size_t bytes_transferred ) {
                        std::istream is( sse_response_.get() );
                        std::string line;
                        std::string::size_type pos(0);
                        while ( std::getline( is, line ) && line != "\r" ) {
                            ADDEBUG() << line;
                            if ( (pos = line.find( "Content-Type:") ) != std::string::npos ) {
                                content_type_ = line.substr( line.find( ':', pos ), line.find( '\r', pos ) );
                                // "text/event-stream" || "blob/event-stream"
                            }
                        }
                        do_asio_read( socket, ec );
                    });
            } else {
                ADDEBUG() << ec;
            }
        }

        void do_asio_read( tcp::socket& socket, const boost::system::error_code& ec ) {
            if ( !ec ) {
                boost::asio::async_read_until(
                    socket
                    , *sse_response_
                    , "\r\n\r\n"
                    , [&]( const boost::system::error_code& ec, size_t bytes_transferred ) {
                        boost::asio::streambuf::const_buffers_type b = sse_response_->data();
                        std::string s(boost::asio::buffers_begin(b), boost::asio::buffers_begin(b) + bytes_transferred );

                        if ( auto ev = sse_stream_ << std::move( s ) )
                            handler_( std::move( ev.get() ) );

                        sse_response_->consume( bytes_transferred );
                        do_asio_read( socket, ec );
                    });
            } else {
                ADDEBUG() << ec;
            }
        }

        request_type req_;
        boost::beast::http::response< boost::beast::http::buffer_body > res_;
        sse_stream sse_stream_;
        std::function< void( sse_event_data_t&& ) > handler_;
        std::unique_ptr< boost::asio::streambuf > sse_response_;
        std::string content_type_;
    };

    //////////////////////////////////////////////
    ////////////  boost::beast::http /////////////
    template< typename request_type = boost::beast::http::request< boost::beast::http::empty_body > >
    struct sse_functor_b {
        sse_functor_b( const sse_functor_b& t ) = delete;
        const sse_functor_b& operator = ( const sse_functor_b& t ) = delete;

        sse_functor_b( request_type&& req
                       , std::function< void( sse_event_data_t&& ) > handler )
            : req_( req )
            , parser_( res_ )
            , handler_( handler ) {
            parser_.eager( true );
        }

        //////////////////
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
                        if ( auto ev = sse_stream_
                             << std::string( sbuf_, sizeof(sbuf_) - parser_.get().body().size ) ) {
                            handler_( std::move( ev.get() ) );
                        }
                        do_read( socket, _ec );
                    });
            } else {
                ADDEBUG() <<ec;
            }
        }
        request_type req_;
        boost::beast::flat_buffer buffer_{8192}; // (Must persist between reads)
        boost::beast::http::response< boost::beast::http::buffer_body > res_;
        boost::beast::http::response_parser< boost::beast::http::buffer_body > parser_;
        char sbuf_[ 2048 ];
        sse_stream sse_stream_;
        std::function< void( sse_event_data_t&& ) > handler_;
    };
}
/////////////

namespace adurl {

    class sse_handler::impl {
    public:
        std::unique_ptr< sse_functor_a<> > sse_functor_;
    };
}


sse_handler::~sse_handler()
{
}

sse_handler::sse_handler( boost::asio::io_context& ioc ) : ioc_( ioc )
                                                         , client_( std::make_unique< client >( ioc ) )
                                                         , impl_( std::make_unique< impl >() )
{
}

bool
sse_handler::connect( const std::string& target
                      , const std::string& host
                      , const std::string& port
                      , sse_event_handler_t handler
                      , bool blocking )
{
    namespace http = boost::beast::http;

    handler_ = handler;

    http::request< http::empty_body > req{ http::verb::post, target, 11 };
    req.set( http::field::host, host );
    req.set( http::field::user_agent, BOOST_BEAST_VERSION_STRING );
    req.set( http::field::accept, "text/event-stream" );
    req.prepare_payload();

    impl_->sse_functor_ = std::make_unique< sse_functor_a<> >(
        std::move( req )
        , [&]( sse_event_data_t&& ev ){
            handler_( std::move( ev ) );
        });

    // sse_functor_a<> fn( std::move( req )
    //                   , [&]( sse_event_data_t&& ev ){
    //                       handler_( std::move( ev ) );
    //                   });

    (*client_)( host, port, *(impl_->sse_functor_) );

    if ( blocking )
        ioc_.run();

    return true;
}
