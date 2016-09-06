/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "acqiris_method.hpp"
#include "document.hpp"
#include "tcp_server.hpp"
#include "task.hpp"
#include <QApplication>
#if defined USING_PROTOBUF
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/util/json_util.h>
#endif
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>

namespace po = boost::program_options;

int
main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    po::variables_map vm;
    po::options_description description( "acqiris" );
    {
        description.add_options()
            ( "help,h",    "Display this help message" )
            ( "server",    "Run as server" )
            ( "connect",  po::value< std::string >()->default_value( "localhost" ), "connect to server" )
            ( "port",     po::value< std::string >()->default_value( "8000" ), "aqdrv4 port numer" )
            ( "recv",     po::value< std::string >()->default_value( "0.0.0.0" ), "For IPv4 0.0.0.0, IPv6, try 0::0" )
            ( "save",     po::value< std::string >(), "save method to file" )
            ( "load",     po::value< std::string >(), "load method from file" )
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }
    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    auto m = std::make_shared< aqdrv4::acqiris_method >();
    auto trig = m->mutable_trig();
    auto hor = m->mutable_hor();
    auto ch1 = m->mutable_ch1();
    auto ext = m->mutable_ext();
    
    if ( vm.count( "load" ) ) {
        document::instance()->set_acqiris_method( document::load( vm[ "load" ].as< std::string >() ) );
    }

    if ( vm.count( "server" ) ) {
        
        document::instance()->set_server(
            std::make_unique< aqdrv4::server::tcp_server >( vm["recv"].as< std::string >()
                                                            , vm["port"].as< std::string >() ) );
    } else if ( vm.count( "connect" ) ) {

        // document::instance()->tcp_connect( vm["connect"].as< std::string >(), vm["port"].as< std::string >() );
    }

    document::instance()->set_acqiris_method( m );

    MainWindow w;
    w.resize( 800, 600 );
    w.onInitialUpdate();

    document::instance()->digitizer_initialize();

    w.show();

    a.exec();

    if ( vm.count( "save" ) ) {
        document::save( vm[ "save" ].as< std::string >(), document::instance()->acqiris_method() );
    }
    
}
