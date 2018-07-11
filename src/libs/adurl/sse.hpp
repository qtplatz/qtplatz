// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#pragma once

#include "adurl_global.h"
#include <boost/asio/io_service.hpp>
#include <functional>
#include <vector>
#include <string>

// SSE client for socfpga/dg-httpd server

namespace adurl {

    class client;

    class ADURLSHARED_EXPORT sse {
    public:

        class ADURLSHARED_EXPORT request_timeout : public  std::exception {};
        class ADURLSHARED_EXPORT error_reply : public std::exception {};

        typedef std::function< void( const std::vector< std::pair< std::string, std::string > >&, const std::string ) > callback_type;
        
        ~sse();
        sse( boost::asio::io_service& );
        
        void register_sse_handler( callback_type );
    
        bool connect( const std::string& url, const std::string& server, const std::string& port = "http" );
    
    private:
        boost::asio::io_service& io_context_;  // still using boost-1.62 on armhf based linux
        std::string server_;
        std::string port_;
        std::string url_;
        std::unique_ptr< client > client_;
        callback_type callback_;
        std::vector< std::pair< std::string, std::string > > headers_;
        bool header_complete_; // when empty line detects
        size_t content_length_;
    };
    

    namespace old {
        class ADURLSHARED_EXPORT sse {
        public:

            class ADURLSHARED_EXPORT request_timeout : public  std::exception {};
            class ADURLSHARED_EXPORT error_reply : public std::exception {};

            ~sse();
            sse( const char * server /* = "dg-httpd"*/, const char * path /* = "/dg/ctl?events" */, const char * port = "80" );
        
            void exec( std::function< void( const char * /* event */, const char * /* data */ ) > callback );
            void stop();

        private:
            class impl;
            impl * impl_;
        };
    }
    
}

