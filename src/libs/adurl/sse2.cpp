// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013-2018 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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
#include <algorithm>
#include <cctype>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

using namespace adurl;

sse::~sse()
{
}


sse::sse( boost::asio::io_service& io_context ) : io_context_( io_context )
                                                  , header_complete_( false )
                                                  , content_length_( 0 )
{
}

void
sse::register_sse_handler( callback_type callback )
{
    callback_ = callback;
}

bool
sse::connect( const std::string& url
               , const std::string& server
               , const std::string& port )

{
    server_ = server;
    port_ = port;
    url_ = url;

    auto request = std::make_unique< boost::asio::streambuf >();
    std::ostream request_stream ( request.get() );

    request_stream << "POST " << request::url_encode( url_ ) << " HTTP/1.0\r\n";
    request_stream << "Host: " << server_ << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Content-Type: application/text\r\n";    
    request_stream << "\r\n";

    if ( (client_ = std::make_unique< client >( io_context_, std::move( request ), server, port )) ) {

        client_->connect( [&]( const boost::system::error_code& ec, boost::asio::streambuf& response ){

                std::istream stream( &response );

                if ( ! header_complete_ ) {
                    std::string data;
                    while ( std::getline( stream, data ) ) {

                        auto pos = data.find_first_of( ':' );
                        if ( pos != std::string::npos ) {
                            auto header = std::make_pair( data.substr( 0, pos ), data.substr( pos + 1 ) );
                            header.second = header.second.substr( 0, header.second.find_first_of( "\r\n" ) );
                            headers_.emplace_back( header );
                            // ADDEBUG() << headers_.back();
                            if ( headers_.back().first == "Content-Length" )
                                content_length_ = std::stol( headers_.back().second );
                        }
                        if ( data == "\r" ) {
                            header_complete_ = true;
                            break;
                        }
                    }
                }

                if ( header_complete_ ) {
                    if ( content_length_ == 0 || ( response.size() >= content_length_ ) ) {
                        auto first = boost::asio::buffer_cast< const char * >( response.data() );
                        std::string body( first, first + content_length_ );
                        response.consume( content_length_ );
                        if ( callback_ )
                            callback_( headers_, body );
                        content_length_ = 0;
                        header_complete_ = false;
                        headers_.clear();
                    }
                }
            });
    }
}

