// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2016-2020 MS-Cheminformatics LLC
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

#include <adio/dgprotocols.hpp>
#include <adportable/debug.hpp>
#include <adurl/blob.hpp>
#include <adurl/client.hpp>
#include <adurl/dg.hpp>
#include <adurl/sse.hpp>
#include <boost/asio.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

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
        ( "help,h",    "Display this help message" )
        ( "dg.commit",    po::value< std::string >(), "commit json file to server" )
        ( "dg.status", "read delay/pulse generagor status" )
        ( "dg.json",   "use json format for status" )
        ( "dg.start",  "fsm-start" )
        ( "dg.stop",   "fsm-stop" )
        ( "blob",       po::value< std::string >(), "blob /dataStorage" )
        ( "sse",        po::value< std::string >(), "sse dg|evbox|hv|(any url string)" )
        ( "host",       po::value< std::string >()->default_value( "httpd-dg" ) )
        ( "port",       po::value< std::string >()->default_value( "http" ) )
        ( "args",       po::value< std::vector< std::string > >(),  "url" )
        ;

    po::positional_options_description p;
    p.add( "args",  -1 );
    po::store( po::command_line_parser( argc, argv ).options( description ).positional(p).run(), vm );
    po::notify(vm);

    adurl::client::setDebug_mode( true );

    if ( vm.count( "help" ) ) { // || ( vm.count( "args" ) == 0 ) ) {
        std::cout << "Usage: " << argv[ 0 ] << " --host=host --port=http [options]" << std::endl;
        std::cout << description;
        return 0;
    }

    auto host = vm[ "host" ].as< std::string >();
    auto port = vm[ "port" ].as< std::string >();

    adurl::dg dg( host, port );

    if ( vm.count( "dg.status" ) ) {
        std::string json;
        if ( dg.fetch( json ) )
            std::cout << json << std::endl;
        return 0;
    }

    if ( vm.count( "dg.commit" ) ) {
        boost::filesystem::path path( vm[ "dg.commit" ].as< std::string >() );
        if ( boost::filesystem::exists( path ) ) {
            std::ifstream is( path.string() );
            std::string json(std::istreambuf_iterator<char>(is), {});
            dg.commit( std::move( json ) );
        } else {
            ADDEBUG() << path << " not found";
        }
    }

    if ( vm.count( "blob" ) ) {
        std::string url = vm[ "blob" ].as< std::string >();
        std::cout << url << std::endl;
        boost::asio::io_service io_context;
        adurl::blob blob( io_context );

        blob.register_blob_handler( []( const std::vector< std::pair< std::string, std::string > >& headers, const std::string& blob ){
            ADDEBUG() << "handle blob";
            for ( const auto& header: headers )
                ADDEBUG() << header;
            ADDEBUG() << "blob size=" << blob.size() << " \tblob: " << blob;
        });

        blob.connect( url, host, port );
        io_context.run();
        return 0;
    }

    if ( vm.count( "sse" ) ) {
        std::string url = vm[ "sse" ].as< std::string >();
        if ( url == "dg" )
            url = "/dg/ctl$events";
        if ( url == "hv" )
            url = "/hv/api$events";
        if ( url == "evbox" )
            url = "/evbox/api$events";

        std::cout << url << std::endl;
#if 1
        boost::asio::io_context ioc;
        adurl::sse_handler sse( ioc );

        sse.connect( url
                     , host
                     , port
                     , []( adurl::sse_event_data_t&& ev ) {
                         static size_t counts = 0;
                         std::string event, data;
                         int32_t id;
                         std::tie( event, id, data ) = std::move( ev );
                         std::cout << "[" << counts++ << "]" <<
                             "event: " << event << "\tid: " << id << "\tdata: " << data.substr( 0, 120 ) << std::endl;
                     });
#else
        adurl::old::sse sse( host.c_str(), url.c_str() );
        sse.exec( [] ( const char * event, const char * data ) {
            std::cout << "event: " << event << "\t" << "data: " << data << std::endl;
        });
#endif
        std::this_thread::sleep_for( std::chrono::seconds( 10 ) );
    }

    return 0;
}
