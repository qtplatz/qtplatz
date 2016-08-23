// This is a -*- C++ -*- header.
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
// async_client.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "adurl_global.h"
#include <boost/asio.hpp>
#include <memory>

using boost::asio::ip::tcp;

namespace adurl {

    class request;

    class ADURLSHARED_EXPORT client {
    public:
        client( boost::asio::io_service& io_service, const std::string& server, const std::string& path );
        client( boost::asio::io_service& io_service, const std::string& server, std::unique_ptr< boost::asio::streambuf >&& request );

        boost::asio::streambuf& response();
        boost::asio::streambuf& response_header();
        unsigned int status_code() const;
        const std::string& status_message() const;
        const boost::system::error_code& error_code() const;

        void connect( std::function< void( const boost::system::error_code&, boost::asio::streambuf& ) > );

        enum ReplyStatus { NoError, Error };
        ReplyStatus error() const;

        static void setDebug_mode( bool mode );
        static bool debug_mode();

    private:
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
        std::unique_ptr< boost::asio::streambuf > request_;
        boost::asio::streambuf response_;
        boost::asio::streambuf response_header_;
        unsigned int status_code_;
        std::string  status_message_;
        std::string  http_version_;
        boost::system::error_code error_code_;
        ReplyStatus error_;
        bool event_stream_;
        std::function< void( const boost::system::error_code&, boost::asio::streambuf& ) > event_stream_handler_;
        std::string server_;
    };
    
}
