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
// This module was derived from:
// async_client.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "acqrscontrols_global.hpp"
#include <boost/asio.hpp>
#include <memory>

using boost::asio::ip::tcp;

namespace acqrscontrols { namespace aqdrv4 { class acqiris_protocol; } }

class request;

namespace acqrscontrols {
namespace aqdrv4 {

    class ACQRSCONTROLSSHARED_EXPORT acqiris_client {
    public:
        acqiris_client( const std::string& server, const std::string& path );
        
        void run();
        void stop();
        
        void readData();
        
        boost::asio::streambuf& response();
        boost::asio::streambuf& response_header();
        
        void connect( std::function< void( const struct preamble *, const char *, size_t ) > );
        
        void write( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_protocol > );
        
    private:
        void handle_resolve( const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator );
        void handle_connect( const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator );
        void handle_write_request( const boost::system::error_code& err );
        void handle_read_status_line( const boost::system::error_code& err );
        void handle_read_headers( const boost::system::error_code& err );
        void handle_read_content( const boost::system::error_code& err );
        void handle_read_stream( const boost::system::error_code& err );
        void do_read();
        void handle_response( const boost::asio::streambuf& );

        boost::asio::io_service io_service_;
        tcp::resolver resolver_;
        tcp::socket socket_;
        std::unique_ptr< boost::asio::streambuf > request_;
        boost::asio::streambuf response_;
        boost::asio::streambuf response_header_;
        boost::system::error_code error_code_;
        std::function< void( const struct preamble *, const char *, size_t ) > response_handler_;
        std::string server_;
    };
}    
}
