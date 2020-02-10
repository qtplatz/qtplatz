// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2016-2020 MS-Cheminformatics LLC
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
// async_client.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "adurl_global.h"
#include <adportable/debug.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/optional.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <functional>

#define ENABLE_BEAST 1

using boost::asio::ip::tcp;
namespace http = boost::beast::http;

namespace adurl {

    class request;
    class ajax;

    template< typename request_type >
    struct request_functor {
        request_functor( const request_functor& t ) = delete;
        const request_functor& operator = ( const request_functor& t ) = delete;

        request_functor( request_type&& req ) : req_( req ) {
        }

        void operator()( tcp::socket& socket
                         , boost::system::error_code& errc ) {
            ADDEBUG() << "---------- request_functor write ------------";
            boost::beast::http::async_write(
                socket
                , req_
                , [&]( const boost::system::error_code& ec, size_t ) {
                    errc = ec;
                    if ( !ec ) {
                        ADDEBUG() << "---------- request_functor read ------------";
                        boost::beast::http::async_read(
                            socket, buffer_, res_
                            , [&]( const boost::system::error_code& ec, size_t ){
                                ADDEBUG() << "---------- request_functor read: " << ec;
                                errc = ec;
                            });
                    }
                });
        } // operator
        request_type req_;
        boost::beast::http::response<boost::beast::http::string_body> res_;
        boost::beast::flat_buffer buffer_; // (Must persist between reads)
    };

    class client {
    public:
        client( boost::asio::io_service& io_service
                , const std::string& path
                , const std::string& server
                , const std::string& port );

        client( boost::asio::io_service& io_service
                , std::unique_ptr< boost::asio::streambuf >&& request
                , const std::string& server
                , const std::string& port = "http" );

        boost::asio::streambuf& response();
        boost::asio::streambuf& response_header();

        unsigned int status_code() const;
        const std::string& status_message() const;
        const boost::system::error_code& error_code() const { return error_code_; }

#if ENABLE_BEAST
        //////////////////////////////////////// ctor
        client( boost::asio::io_service& io_service );

        ////////////////////////////////////////
    private:
        template< typename functor >
        void handle_connect( const boost::system::error_code& ec, tcp::resolver::iterator it, functor& fn ) {
            if ( !ec ) {
                fn(socket_, error_code_ );
            } else if ( it != tcp::resolver::iterator() ) {
                socket_.close();
                tcp::endpoint endpoint = *it;
                auto next = ++it;
                socket_.async_connect( endpoint
                                       , [endpoint,this,next,&fn]( const boost::system::error_code& ec ) {
                                           handle_connect( ec, next, fn );
                                       });
            } else {
                error_code_ = ec;
            }
        }

        template< typename functor >
        void handle_resolve( const boost::system::error_code& ec, tcp::resolver::iterator it, functor& f ) {
            if ( !ec ) {
                tcp::endpoint endpoint = *it;
                auto next = ++it;
                socket_.async_connect( endpoint
                                       , [endpoint,this,next,&f]( const boost::system::error_code& ec ) {
                                           handle_connect( ec, next, f );
                                       });
            } else {
                error_code_ = ec;
            }
        }

    public:
        ////////////////////////
        template< typename body_type >
        boost::optional< boost::beast::http::response< boost::beast::http::string_body > >
        operator()( const std::string& host
                    , const std::string& port
                    , boost::beast::http::request< body_type >&& req ) {

            request_functor< boost::beast::http::request< body_type > > functor( std::move( req ) );
            resolver_.async_resolve( host, port
                                     , [&]( const boost::system::error_code& ec, tcp::resolver::iterator it ){
                                         error_code_ = ec;
                                         handle_resolve( ec, it, functor );
                                     });

            socket_.get_io_context().run(); // block until response complete

            if ( error_code_ )
                return boost::none;
            return std::move( functor.res_ );
        }

        ////////////////////////
        template< typename functor_type >
        void
        operator()( const std::string& host
                    , const std::string& port
                    , functor_type& functor ) {
            //ADDEBUG() << "---------- start functor sesion ------------";
            resolver_.async_resolve( host, port
                                     , [&]( const boost::system::error_code& ec, tcp::resolver::iterator it ){
                                         handle_resolve( ec, it, functor );
                                     });
        }
#endif

        void connect( std::function< void( const boost::system::error_code&, boost::asio::streambuf& ) > );

        enum ReplyStatus { NoError, Error };
        ReplyStatus error() const;

        static void setDebug_mode( bool mode );
        static bool debug_mode();

    private:
        friend class ajax;

        void handle_resolve( const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator );
        void handle_connect( const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator );
        void handle_write_request( const boost::system::error_code& err );
        void handle_read_status_line( const boost::system::error_code& err );
        void handle_read_headers( const boost::system::error_code& err );
        void handle_read_content( const boost::system::error_code& err );
        void handle_read_stream( const boost::system::error_code& err );

        static bool debug_mode_;

        tcp::resolver resolver_;
        tcp::socket socket_;
//#if ! ENABLE_BEAST
        std::unique_ptr< boost::asio::streambuf > request_;
        std::unique_ptr< boost::asio::streambuf > response_;
        boost::asio::streambuf response_header_;
//#endif
        unsigned int status_code_;
        std::string  status_message_;
        std::string  http_version_;
        boost::system::error_code error_code_;
        ReplyStatus error_;
        bool event_stream_;
        std::function< void( const boost::system::error_code&, boost::asio::streambuf& ) > event_stream_handler_;
        std::string server_;
        //std::string port_;
#if ENABLE_BEAST
        boost::optional< boost::beast::http::request<boost::beast::http::empty_body> > req_; // will be deprecated
        boost::beast::http::response<boost::beast::http::string_body> res_;
        boost::beast::flat_buffer buffer_; // (Must persist between reads)
#endif
    };

}
