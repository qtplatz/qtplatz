// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
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

#include "dg.hpp"
#include "ajax.hpp"
#include "client.hpp"
#include "request.hpp"
#include <adportable/debug.hpp>
#include <adio/dgprotocols.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <iostream>

using namespace adurl;

dg::dg( const char * server ) : server_( server )
                              , port_( "http" )
                              , errorState_( false )
{
    auto pos = server_.find( ':' );
    if ( pos != std::string::npos ) {
        port_ = server_.substr( pos );
        server_ = server_.substr( 0, pos - 1 );
    }
}

dg::dg( const std::string& host
        , const std::string& port ) : server_( host )
                                    , port_( port )
                                    , errorState_( false )
{
}

void
dg::resetError()
{
    errorState_ = false;
}

bool
dg::fetch( adio::dg::protocols<adio::dg::protocol<> >& p )
{
    return false;
}

bool
dg::fetch( std::string& json )
{
    if ( auto res = ajax( server_, port_ )( "POST", "/dg/ctl$status.json", "application/json" ) ) {
        json = res.get().body();
        return res.get().result_int() == 200;
    }
    return false;
}

bool
dg::commit( const adio::dg::protocols<adio::dg::protocol<> > & p )
{
    std::ostringstream json;
    adio::dg::protocols<>::write_json( json, p );
    return false;
}

bool
dg::commit( std::string&& json )
{
    if ( auto res = ajax( server_, port_ )( "POST", "/dg/ctl$commit", std::move( json ), "application/json" ) ) {
        ADDEBUG() << res.get();
        return res.get().result_int() == 200;
    }
    return false;
}
