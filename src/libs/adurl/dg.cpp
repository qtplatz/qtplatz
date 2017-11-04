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

#include "dg.hpp"
#include "client.hpp"
#include "request.hpp"
#include <adportable/debug.hpp>
#include <adio/dgprotocols.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <iostream>

using namespace adurl;

dg::dg( const char * server ) : server_( server )
                              , dirty_( true )
                              , errorState_( false )
{
}

bool
dg::start_triggers()
{
    auto request = std::make_unique< boost::asio::streambuf >();
    std::ostream request_stream ( request.get() );

    request_stream << "POST /dg/ctl?fsm=start HTTP/1.0\r\n";
    request_stream << "Host: " << server_ << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n";
    request_stream << "Content-Type: application/text\r\n";    
    request_stream << "\r\n";
    
    boost::asio::io_service io_service;
    adurl::client c( io_service, std::move( request ), server_ );

    io_service.run();

    if ( adurl::client::debug_mode() && c.status_code() != 200 ) {        
        std::cerr << &c.response_header();
        std::cerr << "status_code: " << c.status_code() << ", " << c.status_message() << std::endl;
    }

    return c.error() == adurl::client::NoError && c.status_code() == 200;
}

bool
dg::stop_triggers()
{
    auto request = std::make_unique< boost::asio::streambuf >();
    std::ostream request_stream ( request.get() );

    request_stream << "POST /dg/ctl?fsm=stop HTTP/1.0\r\n";
    request_stream << "Host: " << server_ << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n";
    request_stream << "Content-Type: application/text\r\n";    
    request_stream << "\r\n";
    
    boost::asio::io_service io_service;
    adurl::client c( io_service, std::move( request ), server_ );

    io_service.run();

    if ( adurl::client::debug_mode() && c.status_code() != 200 ) {
        std::cerr << &c.response_header();
        std::cerr << "status_code: " << c.status_code() << ", " << c.status_message() << std::endl;
    }

    return c.error() == adurl::client::NoError && c.status_code() == 200;    
}

void
dg::resetError()
{
    errorState_ = false;
}

bool
dg::fetch( adio::dg::protocols<adio::dg::protocol<> >& p )
{
    boost::asio::io_service io_service;

    adurl::client c( io_service, "/dg/ctl?status.json", server_, "http" );

    io_service.run();

    if ( c.error() == adurl::client::NoError ) {

        std::istream is( &c.response() );
        return p.read_json( is, p );

    }

    return false;
}

bool
dg::fetch( std::string& json )
{
    boost::asio::io_service io_service;

    adurl::client c( io_service, "/dg/ctl?status.json", server_, "http" );

    io_service.run();

    if ( adurl::client::debug_mode() && c.status_code() != 200 ) {
        std::cerr << "-----------------------------------" << std::endl;
        std::cerr << &c.response_header();
        std::cerr << "status_code: " << c.status_code() << ", " << c.status_message() << std::endl;
        std::cerr << "-----------------------------------" << std::endl;
    }

    if ( c.error() == adurl::client::NoError ) {
        auto bufs = c.response().data();
        json = std::string( boost::asio::buffers_begin( bufs ), boost::asio::buffers_begin( bufs ) + c.response().size() );
    }

    return c.error() == adurl::client::NoError && c.status_code() == 200;
}

bool
dg::commit( const adio::dg::protocols<adio::dg::protocol<> > & p )
{
    auto request = std::make_unique< boost::asio::streambuf >();
    std::ostream request_stream ( request.get() );

    std::ostringstream json;
    adio::dg::protocols<>::write_json( json, p );
    
    request_stream << "POST " << "/dg/ctl?commit.json=" << request::url_encode( json.str() ) << " HTTP/1.0\r\n";
    request_stream << "Host: " << server_ << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n";
    request_stream << "Content-Type: application/json\r\n";
    request_stream << "\r\n";
    
    boost::asio::io_service io_service;

    adurl::client c( io_service, std::move( request ), server_ );
    
    io_service.run();

    //if ( adurl::client::debug_mode && c.status_code() != 200 ) {
    if ( adurl::client::debug_mode() ) {
        std::cerr << "-----------------------------------" << std::endl;
        std::cerr << &c.response_header();
        std::cerr << "status_code: " << c.status_code() << ", " << c.status_message() << std::endl;
        std::cerr << "-----------------------------------" << std::endl;
    }

    return c.error() == adurl::client::NoError && c.status_code() == 200;
}
        
