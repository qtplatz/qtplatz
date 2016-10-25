// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC
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
#include "client.hpp"
#include "dg.hpp"
#include "sse.hpp"
#include <adio/dgprotocols.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <streambuf>
#include <thread>

namespace po = boost::program_options;

void
print( const boost::property_tree::ptree& pt )
{
    using boost::property_tree::ptree;

    ptree::const_iterator end = pt.end();
    for (ptree::const_iterator it = pt.begin(); it != end; ++it) {
        std::cout << it->first << ": " << it->second.get_value<std::string>() << std::endl;
        print(it->second);
    }    
}

int
main( int argc, char* argv[] )
{
    po::variables_map vm;
    po::options_description description( argv[0] );

    description.add_options()
        ( "commit",    po::value< std::string >(), "commit json file to server" )
        ( "help,h",    "Display this help message" )
        ( "status",    "read status from server" )
        ( "json",      "use json format for status" )
        ( "start",     "fsm-start" )
        ( "stop",      "fsm-stop" )
        ( "sse",       "sse debug" )
        ( "args",       po::value< std::vector< std::string > >(),  "host" )
        ;
    
    po::positional_options_description p;
    p.add( "args",  -1 );
    po::store( po::command_line_parser( argc, argv ).options( description ).positional(p).run(), vm );
    po::notify(vm);

    adurl::client::setDebug_mode( true );

    if ( vm.count( "help" ) || ( vm.count( "args" ) == 0 ) ) {
        std::cout << "Usage: " << argv[ 0 ] << "\n\thost[:port] [options]" << std::endl;        
        std::cout << description;
        return 0;
    }

    for ( auto& host: vm[ "args" ].as< std::vector< std::string > >() ) {

        adurl::dg dg( host.c_str() );

        if ( vm.count( "status" ) ) {

            if ( vm.count( "json" ) ) {
                std::string json;
                if ( dg.fetch( json ) )
                    std::cout << json << std::endl;

            } else {
                adio::dg::protocols< adio::dg::protocol<> > proto;
                if ( dg.fetch( proto ) ) {
                    std::cout << boost::format( "interval: %.3le (s)" ) % proto.interval() << std::endl;
                    
                    for ( auto& p: proto ) {
                        std::cout << boost::format( "replicates: %1%" ) % p.replicates() << std::endl;
                        for ( auto& pulse: p.pulses() ) {
                            std::cout << boost::format( "{%.2lf, %.2lf}, " )
                                % ( pulse.first * 1.0e6 ) % ( pulse.second * 1.0e6 );
                        }
                        std::cout << ("in microseconds") << std::endl;
                    }
                }
            }
        }
        if ( vm.count( "commit" ) ) {

            std::ifstream json( vm[ "commit" ].as< std::string >() );
            adio::dg::protocols< adio::dg::protocol<> > protocols;
            try {
                if ( protocols.read_json( json, protocols ) ) {
                    dg.commit( protocols );
                }
            } catch ( std::exception& e ) {
                std::cerr << boost::diagnostic_information( e );
            }
        }

        if ( vm.count( "start" ) ) {
            dg.start_triggers();
        }

        if ( vm.count( "stop" ) ) {
            dg.stop_triggers();
        }        

        if ( vm.count( "sse" ) ) {

            adurl::sse sse( host.c_str(), "/dg/ctl?events" );

            sse.exec( [] ( const char * event, const char * data ) {
                    std::cout << "event: " << event << "\t" << "data: " << data << std::endl;
                });

            std::this_thread::sleep_for( std::chrono::seconds( 10 ) );
            sse.stop();
        }
    }

    return 0;
}
