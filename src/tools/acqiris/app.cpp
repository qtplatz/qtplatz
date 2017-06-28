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

#if defined __APPLE__
# include <compiler/disable_deprecated.h>
#endif
#include "app.hpp"
#include "mainwindow.hpp"
#include "document.hpp"
#include "tcp_server.hpp"
#include "task.hpp"
#include <acqrscontrols/acqiris_method.hpp>
#include <acqrscontrols/acqiris_client.hpp>
#include <QApplication>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <future>

namespace po = boost::program_options;

int
app::main( int argc, char * argv [] )
{
    QApplication a( argc, argv );
    
    po::variables_map vm;
    po::options_description description( "acqiris" );
    {
        description.add_options()
            ( "help,h",    "Display this help message" )
            ( "server",    "Run as server" )
            ( "port",      po::value< std::string >()->default_value( "8010" ),  "aqdrv4 port numer" )
            ( "connect",   po::value< std::string >(), "connect to server (connext=nipxi)" )
            ( "recv",      po::value< std::string >()->default_value( "0.0.0.0" ), "For IPv4 0.0.0.0, IPv6, try 0::0" )
            ( "load",      po::value< std::string >(), "load method from file" )
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }
    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    if ( vm.count( "server" ) && vm.count( "connect" ) ) {
        std::cout << "server and connect are exclusive" << std::endl;
        return 0;
    }
    
    MainWindow w;
    w.resize( 800, 600 );
    w.onInitialUpdate();
    w.show();

    QCoreApplication::processEvents();

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


#if HAVE_AqDrv4 // no AP240/DCnnn hardware driver installed on the compiling host
    auto future = std::async( std::launch::async, [&]() {
            if ( ! isClient ) {

                task::instance()->connect_acqiris_method_adapted(
                    [&]( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > adapted ) {
                        document::instance()->acqiris_method_adapted( adapted ); // update local ui
                    });

                task::instance()->connect_push( [&]( std::shared_ptr< acqrscontrols::aqdrv4::waveform > p ){
                        document::instance()->push( p );
                    });

                task::instance()->connect_replyTemperature( [&]( int temp ){
                        document::instance()->replyTemperature( temp );
                    });

                document::instance()->connect_prepare( boost::bind( &task::prepare_for_run, task::instance(), _1, _2 ) );
                document::instance()->connect_event_out( boost::bind( &task::event_out, task::instance(), _1 ) );
                document::instance()->connect_finalize( boost::bind( &task::finalize, task::instance() ) );
        
                if ( task::instance()->digitizer_initialize() )
                    task::instance()->prepare_for_run( document::instance()->acqiris_method(), acqrscontrols::aqdrv4::allMethod );
            }
        } );
#endif

    a.exec();

    future.get();

    return 0;
}

