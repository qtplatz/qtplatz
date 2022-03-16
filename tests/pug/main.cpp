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

//#include "mainwindow.hpp"
//#include "document.hpp"
#include "http_client_async.hpp"
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

// https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/2244

    po::variables_map vm;
    po::options_description description( argv[0] );
    {
        description.add_options()
            ( "help,h",    "Display this help message" )
            //( "host,h",    po::value< std::string >()->default_value( "www.example.com" ), "host" )
            ( "host,h",    po::value< std::string >()->default_value( "pubchem.ncbi.nlm.nih.gov" ), "host" )
            ( "port,p",    po::value< std::string >()->default_value( "https" ), "port" )
            ( "target,t",  po::value< std::string >()->default_value( "/rest/pug/compound/cid/2244" ),  "target" )
            // ( "target,t",  po::value< std::string >()->default_value( "/" ),    "target" )
            ( "version,v", po::value< std::string >()->default_value( "1.0" ),  "version" )
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }
    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    auto const host = vm[ "host" ].as< std::string >().c_str();
    auto const port = vm[ "port" ].as< std::string >().c_str();
    auto const target = vm[ "target" ].as< std::string >().c_str();
    int version = vm[ "version" ].as< std::string >() == "1.0" ? 10 : 11;  // "1.0"

    // The io_context is required for all I/O
    boost::asio::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};
    load_root_certificates(ctx);

    std::make_shared<session>( boost::asio::make_strand(ioc),  ctx )->run(host, port, target, version);

    // std::make_shared<session>(ioc)->run( host.c_str(), port.c_str(), target.c_str(), version );
    ioc.run();

    return 0;
}
