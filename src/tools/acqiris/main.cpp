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
#if ! defined DAEMON
#include "mainwindow.hpp"
#endif
#include "document.hpp"
#include "tcp_server.hpp"
#include "task.hpp"
#include <acqrscontrols/acqiris_method.hpp>
#include <acqrscontrols/acqiris_client.hpp>
#if defined DAEMON
# include <QCoreApplication>
#else
# include <QApplication>
#endif
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
#if defined DAEMON
    QCoreApplication a(argc, argv);
#else
    QApplication a(argc, argv);
#endif

    po::variables_map vm;
    po::options_description description( "acqiris" );
    {
        description.add_options()
            ( "help,h",    "Display this help message" )
            ( "server",    "Run as server" )
            ( "port",      po::value< std::string >()->default_value( "8010" ), "aqdrv4 port numer" )
            ( "connect",   po::value< std::string >(), "connect to server" )
            ( "recv",      po::value< std::string >()->default_value( "0.0.0.0" ), "For IPv4 0.0.0.0, IPv6, try 0::0" )
            ( "load",      po::value< std::string >(), "load method from file" )
            ( "daemon",    "No gui" )
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }
    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    document::instance()->initialSetup();
    
    if ( vm.count( "load" ) )
        document::instance()->set_acqiris_method( document::load( vm[ "load" ].as< std::string >() ) );

    bool isClient( false );
    bool isServer( false );

    if ( vm.count( "server" ) ) {

        isServer = true;
        
        document::instance()->set_server(
            std::make_unique< acqiris::server::tcp_server >(
                vm["recv"].as< std::string >(), vm["port"].as< std::string >() ) );

    } else if ( vm.count( "connect" ) ) {

        isClient = true;
        
        document::instance()->set_client(
            std::make_unique< acqrscontrols::aqdrv4::acqiris_client >(
                vm["connect"].as< std::string >(), vm["port"].as< std::string >() ) );
    }

    if ( !isClient )
        task::instance()->digitizer_initialize();

    if ( vm.count( "daemon" ) ) {

        int fd = open( PID_NAME, O_RDWR|O_CREAT, 0644 );
        if ( fd < 0 ) {
            std::cerr << "Can't open " PID_NAME << std::endl;
            exit(1);
        }
        int lock = lockf( fd, F_TLOCK, 0 );
        if ( lock < 0 ) {
            std::cerr << "Process " << argv[0] << " already running" << std::endl;
            exit(1);
        }
        std::ostringstream o;
        o << getpid() << std::endl;
        write( fd, o.str().c_str(), o.str().size() );
        
        return a.exec();

    } else {

#if ! defined DAEMON
        MainWindow w;
        w.resize( 800, 600 );
        w.onInitialUpdate();
        w.show();
#endif
        
        a.exec();
    }
}
