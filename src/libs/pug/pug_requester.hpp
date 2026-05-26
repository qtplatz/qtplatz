/**************************************************************************
** Copyright (C) 2016-2026 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016-2026 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "pug_global.hpp"
#include <pugixml.hpp>
#include <boost/beast/http.hpp>
#include <boost/certify/https_verification.hpp>
#include <cstdlib>
#include <string>

namespace pug {

    struct PUGSHARED_EXPORT pug_handler;
    struct PUGSHARED_EXPORT pug_requester;

    struct pug_handler {
        void operator()( boost::beast::http::response< boost::beast::http::string_body > res, const std::string& target );
    };

    //-----------------------
    struct pug_requester {
        const int version_;
        const std::string host_;
        const std::string port_;
        mutable boost::asio::ssl::context ctx_; //{ boost::asio::ssl::context::tlsv12_client };
        pug_requester( int version
                       , const std::string& host
                       , const std::string& port);

        void operator()( const std::string& target
                         , std::function<void( boost::beast::http::response< boost::beast::http::string_body >
                                               , std::string )> handler = pug_handler{} ) const;
    };

}
