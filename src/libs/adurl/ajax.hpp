// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC
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
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/asio/streambuf.hpp>
#include <array>
#include <memory>
#include <string>

// namespace boost { namespace system { class error_code; } }

namespace adurl {

    class client;

    class ADURLSHARED_EXPORT ajax {
    public:
        ~ajax();
        ajax( const std::string& server = "localhost", const std::string& port = "http" );

        bool operator()( const std::string& method, const std::string& url, const std::string& mimeType = "application/json" );

        bool get_response( boost::property_tree::ptree& ) const;
        std::string response( bool pretty_print = true ) const;
        std::string response_header() const;
        
        unsigned int status_code() const;
        const std::string& status_message() const;

    private:
        std::string server_;
        std::string port_;
        std::string response_header_;
        unsigned int status_code_;
        std::string status_message_;
        std::unique_ptr< boost::asio::streambuf > response_;
    };
    
}
