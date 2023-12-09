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

#include "http_client_async.hpp"
#include <boost/program_options.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/json.hpp>
#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <future>

namespace po = boost::program_options;

int
main( int argc, char * argv [] )
{
    // https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/PFOS/property/CanonicalSMILES,MolecularFormula,InChiKey/JSON
    // https://pubchem.ncbi.nlm.nih.gov/rest/autocomplete/compound/perfuluoroalkylethanol/json?limit=10

    po::variables_map vm;
    po::options_description description( argv[0] );
    {
        description.add_options()
            ( "help,h",    "Display this help message" )
            ( "host",    po::value< std::string >()->default_value( "pubchem.ncbi.nlm.nih.gov" ), "host" )
            ( "port",    po::value< std::string >()->default_value( "https" ), "port" )
            ( "version,v", po::value< std::string >()->default_value( "1.0" ),  "version" )
            ( "domain,d",  po::value< std::string >()->default_value( "compound" ),  "substance|compound|assay|gene|protein|pathway|taxonomy|cell" )
            ( "namespace,n",  po::value< std::string >()->default_value( "name" ),  "cid|name|smiles|inchi|sdf|inchikey|formula")
            ( "identifier,i",  po::value< std::vector< std::string > >()->multitoken(), "query identifier")
            ( "property",      po::value< std::string >()->default_value( "CanonicalSMILES,MolecularFormula,InChiKey" ), "")
            ( "output",      po::value< std::string >()->default_value( "JSON" ), "XML|ASNT|ASNB|JSON|JSONP[?callback=<callback name>]|SDF|CSV|PNG|TXT")
            ( "autocomplete,a", "autocomplete; pug if not specified" )
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }
    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }
    if ( vm.count( "identifier" ) == 0 ) {
        std::cout << description << std::endl;
        std::cout << argv[0] << " specify identifer such as: " << std::endl;
        std::cout << "\tpug --identifier APAP" << std::endl;
        std::cout << "\tpug -a -i perfuluoroalkeylethanol" << std::endl;
        return 0;
    }

    auto const host = vm[ "host" ].as< std::string >().c_str();
    auto const port = vm[ "port" ].as< std::string >().c_str();
    int version = vm[ "version" ].as< std::string >() == "1.0" ? 10 : 11;  // "1.0"

    std::vector< std::string > identifiers = vm[ "identifier" ].as< std::vector< std::string > >();
    bool autocomplete = vm.count( "autocomplete" );

    for ( auto identifier: identifiers ) {

        std::ostringstream o;
        o << "/rest";
        if ( autocomplete ) {
            o << "/autocomplete"
              << "/" << vm[ "domain" ].as< std::string >()             // compound
              << "/" << identifier
              << "/json?limit=20";
        } else {
            o << "/pug"
              << "/" << vm[ "domain" ].as< std::string >()             // compound
              << "/" << vm[ "namespace" ].as< std::string >()          // name
              << "/" << identifier
              << "/property/" << vm[ "property" ].as< std::string >()  // CanonicalSMILES
              << "/" << vm[ "output" ].as< std::string >();             // JSON
        }

        auto target = o.str();

        boost::asio::io_context ioc;
        boost::asio::ssl::context ctx{ boost::asio::ssl::context::tlsv12_client };
#if 0
         load_root_certificates(ctx);
#else
         // verify SSL context
         {
             ctx.set_verify_mode(boost::asio::ssl::verify_peer |
                                 boost::asio::ssl::context::verify_fail_if_no_peer_cert);
             ctx.set_default_verify_paths();
             boost::certify::enable_native_https_server_verification(ctx);
         }
#endif

         auto future = std::make_shared< session >( boost::asio::make_strand(ioc),  ctx )->run(host, port, target.c_str(), version);
        ioc.run();

        auto res = future.get();
        std::cout << "\n## target:\t" << target << std::endl;
        std::cout << "## response: \n\t" << res.body().data();

        boost::system::error_code ec;
        auto jv = boost::json::parse( res.body(), ec );
        if ( !ec ) {
            std::cout << "\n" << jv << std::endl;
        }

    }

    return 0;
}
