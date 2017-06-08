//
// main.cpp
// ~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
/**************************************************************************
** Copyright (C) 2014-2016 Toshinobu Hondo, for delay generator
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
*/

#include "config.h"
#include "log.hpp"
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include "server.hpp"

int __verbose_level__ = 0;
bool __debug_mode__ = false;

int
main(int argc, char* argv[])
{
    namespace po = boost::program_options;
    
    try {
        po::variables_map vm;
        
        po::options_description desc("options");
        desc.add_options()
            ( "help", "print help message" )
            ( "version", "print version number" )            
            ( "port", po::value<std::string>()->default_value("8080"), "http port number" )
            ( "recv", po::value<std::string>()->default_value("0.0.0.0"), "For IPv4, try 0.0.0.0, IPv6, try 0::0" )
            ( "doc_root", po::value<std::string>()->default_value( DOC_ROOT ), "document root" )
            ( "verbose", po::value<int>()->default_value(0), "verbose level" )
            ( "debug,d", "debug mode" )            
            ;
        po::store( po::command_line_parser( argc, argv ).options( desc ).run(), vm );
        po::notify( vm );

        if ( vm.count( "help" ) ) {
            std::cout << desc;
            return 0;
        } else if ( vm.count( "version" ) ) {
            std::cout << PACKAGE_VERSION << std::endl;
            return 0;
        }

        __debug_mode__ = vm.count( "debug" ) > 0 ;
        
#if ! defined WIN32
        if ( ! __debug_mode__ ) {
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
        }
#endif        
        // Initialise the server.
        dg::log() << boost::format( "started on %1% %2% %3%" )
            % vm["recv"].as< std::string >()
            % vm["port"].as< std::string >()
            % vm["doc_root"].as< std::string >();

        __verbose_level__ = vm["verbose"].as< int >();
            
        http::server::server s( vm["recv"].as< std::string >().c_str()
                                , vm["port"].as< std::string >().c_str()
                                , vm["doc_root"].as< std::string >().c_str() );
        
        // Run the server until stopped.
        s.run();
        
    }  catch (std::exception& e)  {
        std::cerr << "exception: " << e.what() << "\n";
    }
    
    return 0;
}
