// This is a -*- C++ -*- header.
/**************************************************************************
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

#pragma once

#include "adurl_global.h"
#include <boost/asio/io_service.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/system/error_code.hpp>
//#include <boost/signals2.hpp>
#include <functional>
#include <vector>
#include <string>

// SSE client for socfpga/dg-httpd server

namespace adurl {

    class ADURLSHARED_EXPORT client;
    class ADURLSHARED_EXPORT sse_handler;
    
    typedef std::tuple< std::string, int32_t, std::string > sse_event_data_t; // event,id,data

    class sse_handler {
    public:
        ~sse_handler();
        sse_handler( boost::asio::io_context& ioc );

        typedef std::function< void( sse_event_data_t&& ) > sse_event_handler_t;

        bool connect( const std::string& url
                      , const std::string& host
                      , const std::string& port
                      , sse_event_handler_t
                      , bool blocking = true );
        void exec();

    private:
        boost::asio::io_context& ioc_;
        std::unique_ptr< client > client_;
        sse_event_handler_t handler_;
        class impl;
        std::unique_ptr< impl > impl_;
    };


}
