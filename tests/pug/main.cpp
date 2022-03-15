/**************************************************************************
** Copyright (C) 2016-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mainwindow.hpp"
#include "document.hpp"
#include <QApplication>
#include <boost/program_options.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <iostream>
#include <fstream>
#include <future>

namespace po = boost::program_options;

int
main( int argc, char * argv [] )
{
    QApplication a( argc, argv );

    po::variables_map vm;
    po::options_description description( argv[0] );
    {
        description.add_options()
            ( "help,h",    "Display this help message" )
            ( "host,h",    po::value< std::string >()->default_value( "www.example.com" ), "host" )
            ( "port,p",    po::value< std::string >()->default_value( "80" ), "port" )
            ( "target,t",  po::value< std::string >()->default_value( "/" ),  "target" )
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }
    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    auto const host = vm[ "host" ].as< std::string >();
    auto const port = vm[ "port" ].as< std::string >();
    auto const target = vm[ "target" ].as< std::string >();
    int version = 10;  // "1.0"

    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // These objects perform our I/O
    boost::asio::ip::tcp::resolver resolver(ioc);
    boost::beast::tcp_stream stream(ioc);

    // Look up the domain name
    auto const results = resolver.resolve(host, port);

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);

    // Set up an HTTP GET request message
    boost::beast::http::request< boost::beast::http::string_body> req{boost::beast::http::verb::get, target, version};

    req.set( boost::beast::http::field::host, host);
    req.set( boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send the HTTP request to the remote host
    boost::beast::http::write(stream, req);

    // This buffer is used for reading and must be persisted
    boost::beast::flat_buffer buffer;

    // Declare a container to hold the response
    boost::beast::http::response< boost::beast::http::dynamic_body > res;

    // Receive the HTTP response
    boost::beast::http::read(stream, buffer, res);

    // Write the message to standard out
    std::cout << res << std::endl;

    // Gracefully close the socket
    boost::beast::error_code ec;
    stream.socket().shutdown( boost::asio::ip::tcp::socket::shutdown_both, ec );

    // not_connected happens sometimes
    // so don't bother reporting it.
    //
    if( ec && ec != boost::beast::errc::not_connected )
        throw boost::beast::system_error{ec};

    // If we get here then the connection is closed gracefully
    /*
    MainWindow w;
    w.resize( 800, 600 );
    w.onInitialUpdate();
    w.show();

    QCoreApplication::processEvents();
    a.exec();
    */
    return 0;
}
