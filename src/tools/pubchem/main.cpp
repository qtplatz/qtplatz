/**************************************************************************
** Copyright (C) 2016-2026 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016-2026 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <adportable/debug.hpp>
#include <adportable/csv_reader.hpp>
#include <pugixml.hpp>
#include <xmlparser/pugiwrapper.hpp>
#include <pug/pug_requester.hpp>
#include <pug/http_client_async.hpp>
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
#include <iomanip>

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
            ( "cas",  po::value< std::string >(),  "CAS number such as '57-14-7'")
            ( "csv",  po::value< std::string >(),  "CSV file contains CAS in column")
            ( "cas_column",  po::value< int >(),  "Column# for CAS in CSV file (start from zero)")
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }

    if ( vm.count( "help" ) ||
         (vm.count( "identifier" ) == 0 &&
          vm.count( "cas") == 0 &&
          vm.count( "csv" ) == 0) ) {
        std::cout << description << std::endl;
        std::cout << argv[0] << " specify identifer such as: " << std::endl;
        std::cout << "\tpug --identifier APAP" << std::endl;
        std::cout << "\tpug -a -i perfuluoroalkeylethanol" << std::endl;
        return 0;
    }

    auto const host = vm[ "host" ].as< std::string >().c_str();
    auto const port = vm[ "port" ].as< std::string >().c_str();
    int version = vm[ "version" ].as< std::string >() == "1.0" ? 10 : 11;  // "1.0"

    pug::pug_requester pug( version, host, port );

    if ( vm.count( "identifier" ) ) {
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
            pug( target );
        }
    }

    if ( vm.count( "cas" ) ) {

        struct pug_cas_handler {
            std::optional< int >& cid_;
            pug_cas_handler( std::optional< int >& cid ) : cid_( cid ) {}
            void operator()( boost::beast::http::response< boost::beast::http::string_body > res
                             , const std::string& target ) {
                if ( target.contains( "cids/XML" ) ) {
                    pugi::xml_document doc;
                    if ( auto result = doc.load_string( res.body().data(), res.body().size() ) ) {
                        if ( auto node = doc.select_node( "/IdentifierList/CID" ) ) {
                            node.node().print( std::cout );
                            cid_ = node.node().text().as_int();
                        }
                    }
                } else {
                    ADDEBUG() << "## response: \n\t" << res.body().data();
                }
            }
        };

        std::optional< int > cid;

        auto target = std::format("/rest/pug/compound/name/{}/cids/XML", vm["cas"].as< std::string >() );
        pug( target, pug_cas_handler(cid) );

        if ( cid ) {
            auto target = std::format("/rest/pug/compound/cid/{}/property/Title,CanonicalSMILES,MolecularFormula,ExactMass/JSON", *cid );
            pug( target );
        } else {
            ADDEBUG() << "cid not found.";
        }
    }

    if ( vm.count( "csv" ) && vm.count( "cas_column" ) ) {
        const int cas_column = vm["cas_column"].as<int>();
        auto filename = vm["csv"].as< std::string >();
        std::ifstream file;
        if ( filename != "-" )
            file.open( filename );
        std::istream& inf = (filename == "-") ? std::cin : file;
        adportable::csv::csv_reader reader( inf );
        adportable::csv::list_type list;

        while ( reader.read( list ) && inf.good() ) {
            if ( list.size() > cas_column &&
                 list.at( cas_column ).type() == typeid( std::string ) ) {
                auto cas = boost::get<std::string >(list.at( cas_column ) );
                std::ostringstream o;
                for ( const auto& value: list )
                    o << value << "\t";
                ADDEBUG() << "cas# " << cas;
                ADDEBUG() << o.str();
            }
        }
    }

    return 0;
}
