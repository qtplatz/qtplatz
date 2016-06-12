// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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
#include <algorithm>
#include <cctype>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

namespace adurl {

    class sse::impl {

    public:

        impl( std::unique_ptr< boost::asio::streambuf >&& request
              , const std::string& server ) : server_( server )
                                            , work_( io_service_ )
                                            , client_( io_service_, server, std::move( request ) ) {

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
    
}

using namespace adurl;

sse::~sse()
{
    if ( !impl_->threads_.empty() )
        stop();

    delete impl_;
}


sse::sse( const char * server, const char * path ) // : impl_( new impl( server, path ) )
{
    auto request = std::make_unique< boost::asio::streambuf >();
    std::ostream request_stream ( request.get() );

    request_stream << "POST " << request::url_encode( path ) << " HTTP/1.0\r\n";
    request_stream << "Host: " << server << "\r\n";
    request_stream << "Accept: */*\r\n";
    // request_stream << "Connection: close\r\n";
    request_stream << "Content-Type: application/text\r\n";    
    request_stream << "\r\n";
    
    impl_ = new impl( std::move( request ), server );
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

