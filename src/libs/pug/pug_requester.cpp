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

#include "pug_requester.hpp"
#include <adportable/debug.hpp>
#include <pugixml.hpp>
#include <xmlparser/pugiwrapper.hpp>
#include <pug/http_client_async.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/json.hpp>
#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <future>

namespace pug {

    void pug_handler::operator()( boost::beast::http::response< boost::beast::http::string_body > res
                                  , const std::string& target )
    {
        std::cout << std::format( "\ntarget:\t{}", target ) << std::endl;
        if ( target.contains( "JSON" ) ) {
            boost::system::error_code ec;
            auto jv = boost::json::parse( res.body(), ec );
            if ( !ec ) {
                std::cout << "\n" << jv << std::endl;
            } else {
                std::cout << "\n" << ec << std::endl;
            }
        } else if ( target.contains( "XML" ) ) {
            pugi::xml_document doc;
            if ( auto result = doc.load_buffer( res.body().data(), res.body().size() ) ) {
                doc.print( std::cout );
                std::cout << std::endl;
            }
        } else
            std::cout << res.body().data() << std::endl;
    }


    //-----------------------
    // struct pug_requester {
    //     const int version_;
    //     const std::string host_;
    //     const std::string port_;
    //     mutable boost::asio::ssl::context ctx_; //{ boost::asio::ssl::context::tlsv12_client };
    pug_requester::pug_requester( int version
                                  , const std::string& host
                                  , const std::string& port) : version_( version )
        , host_( host )
        , port_( port )
        , ctx_( boost::asio::ssl::context::tlsv12_client )
    {
        // verify SSL context
        ctx_.set_verify_mode(boost::asio::ssl::verify_peer |
                             boost::asio::ssl::context::verify_fail_if_no_peer_cert);
        ctx_.set_default_verify_paths();
        boost::certify::enable_native_https_server_verification(ctx_);
    }

    void pug_requester::operator()( const std::string& target
                                    , std::function<void( boost::beast::http::response< boost::beast::http::string_body >
                                                          , std::string )> handler ) const
    {
        boost::asio::io_context ioc;
        auto future = std::make_shared< session >( boost::asio::make_strand(ioc)
                                                   ,  ctx_ )->run(host_
                                                                  , port_
                                                                  , target.c_str()
                                                                  , version_ );
        ioc.run();

        auto res = future.get();
        handler( res, target );
    }

} // namespace pubchem
