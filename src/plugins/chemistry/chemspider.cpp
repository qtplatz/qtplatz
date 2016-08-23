/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "chemspider.hpp"
#include <adurl/client.hpp>
#include <adportable/debug.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <string>

using namespace chemistry;

static void
print( std::ostream& outf, const boost::property_tree::ptree& pt )
{
    using boost::property_tree::ptree;

    ptree::const_iterator end = pt.end();
    for (ptree::const_iterator it = pt.begin(); it != end; ++it) {
        outf << it->first << ": " << it->second.get_value<std::string>() << std::endl;
        print( outf, it->second );
    }    
}

// reference: http://www.chemspider.com/search.asmx
static const char * logfile = "csdebug.txt";

ChemSpider::ChemSpider()
{
}

ChemSpider::~ChemSpider()
{
}

bool
ChemSpider::query( const std::string& stmt, const std::string& token )
{
    auto request = std::make_unique< boost::asio::streambuf >();
    std::ostream request_stream ( request.get() );

    boost::property_tree::ptree pt;

    // SOAP 1.2
    pt.add( "soap12:Envelope.soap12:Body.AsyncSimpleSearch.query", stmt );
    pt.add( "soap12:Envelope.soap12:Body.AsyncSimpleSearch.token", token );

    auto& envelope = pt.get_child( "soap12:Envelope" );
    envelope.put( "<xmlattr>.xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
    envelope.put( "<xmlattr>.xmlns:xsd", "http://www.w3.org/2001/XMLSchema" );
    envelope.put( "<xmlattr>.xmlns:soap12", "http://www.w3.org/2003/05/soap-envelope" );

    auto& search = envelope.get_child( "soap12:Body.AsyncSimpleSearch" );
    search.put( "<xmlattr>.xmlns", "http://www.chemspider.com/" );
    
    using namespace boost::property_tree;

    std::ostringstream xml;
    xml_parser::write_xml( xml, pt, boost::property_tree::xml_writer_make_settings<std::string>( ' ', 2 ) );

    // debug
    {
        std::ofstream of( logfile, std::ios_base::out );
        of << "--------------- query --------------" << std::endl;
        of << xml.str() << std::endl;
    }

    request_stream << "POST /search.asmx HTTP/1.1\r\n";
    request_stream << "Host: www.chemspider.com\r\n";
    request_stream << "Content-Type: application/soap+xml; charset=utf-8\r\n";
    request_stream << "Content-Length: " << xml.str().length() << "\r\n";
    request_stream << "Connection: close\r\n\r\n";
    request_stream << xml.str();

    boost::asio::io_service io_service;

    adurl::client cs( io_service, "www.chemspider.com", std::move( request ) );

    io_service.run();

    if ( cs.status_code() != 200 ) {
        ADDEBUG() << &cs.response_header();
        ADDEBUG() << "status_code: " << cs.status_code() << ", " << cs.status_message();
    } else {
        std::istream response_stream( &cs.response() );

        boost::property_tree::ptree rpt;

        xml_parser::read_xml( response_stream, rpt );
        {
            std::ofstream of( logfile, std::ios_base::out | std::ios_base::app );
            of << ">>>>>>>> search response >>>>>>>>>>>>" << std::endl;
            xml_parser::write_xml( of, rpt, boost::property_tree::xml_writer_make_settings<std::string>( ' ', 2 ) );
        }

        // ChemSpider always return SOAP 1.1 format
        if ( boost::optional< ptree& > resp = rpt.get_child_optional( "soap:Envelope.soap:Body.AsyncSimpleSearchResponse.AsyncSimpleSearchResult" ) ) {
            rid_ = resp.get().data();
            return getAsyncSearchResult( rid_, token );
        } else if ( boost::optional< ptree& > resp = rpt.get_child_optional( "soap12:Envelope.soap12:Body.AsyncSimpleSearchResponse.AsyncSimpleSearchResult" ) ) {
            rid_ = resp.get().data();
            return getAsyncSearchResult( rid_, token );
        } else {
            ADDEBUG() << "AsyncSimpleSearchResult node not found";
        }
    }
    return false;
}

bool
ChemSpider::getAsyncSearchResult( const std::string& rid, const std::string& token )
{
    using namespace boost::property_tree;

    std::ostringstream xml;

    do {
        boost::property_tree::ptree pt;
    
        pt.add( "soap12:Envelope.soap12:Body.GetAsyncSearchResult.rid", rid );
        pt.add( "soap12:Envelope.soap12:Body.GetAsyncSearchResult.token", token );
        
        auto& envelope = pt.get_child( "soap12:Envelope" );
        envelope.put( "<xmlattr>.xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
        envelope.put( "<xmlattr>.xmlns:xsd", "http://www.w3.org/2001/XMLSchema" );
        envelope.put( "<xmlattr>.xmlns:soap12", "http://www.w3.org/2003/05/soap-envelope" );
        
        auto& result = envelope.get_child( "soap12:Body.GetAsyncSearchResult" );
        result.put( "<xmlattr>.xmlns", "http://www.chemspider.com/" );
    
        xml_parser::write_xml( xml, pt, boost::property_tree::xml_writer_make_settings<std::string>( ' ', 2 ) );
    } while ( 0 );

    // debug
    { std::ofstream of( logfile, std::ios_base::out | std::ios_base::app );
        of << "------------ getAsyncSearchResult -------------" << std::endl;
        of << xml.str() << std::endl; }

    auto request = std::make_unique< boost::asio::streambuf >();
    std::ostream request_stream ( request.get() );

    request_stream << "POST /search.asmx HTTP/1.1\r\n";
    request_stream << "Host: www.chemspider.com\r\n";
    request_stream << "Content-Type: application/soap+xml; charset=utf-8\r\n";
    request_stream << "Content-Length: " << xml.str().length() << "\r\n";
    request_stream << "Connection: close\r\n\r\n";
    request_stream << xml.str();
    
    boost::asio::io_service io_service;
    adurl::client cs( io_service, "www.chemspider.com", std::move( request ) );

    io_service.run();

    if ( cs.status_code() != 200 ) {
        ADDEBUG() << &cs.response_header();
        ADDEBUG() << "status_code: " << cs.status_code() << ", " << cs.status_message();
    } else {
        std::istream response_stream( &cs.response() );

        boost::property_tree::ptree pt;
        xml_parser::read_xml( response_stream, pt );

        // debug
        { std::ofstream of( logfile, std::ios_base::out | std::ios_base::app );
            xml_parser::write_xml( of, pt, boost::property_tree::xml_writer_make_settings<std::string>( ' ', 2 ) );
            of << "---------------------------" << std::endl;
            print( of, pt );
        }

        int i = 0;
        for( auto& child : pt.get_child( "soap:Envelope.soap:Body.GetAsyncSearchResultResponse.GetAsyncSearchResultResult" ) ) {
            int csid = boost::lexical_cast< int >( child.second.data() );
            ADDEBUG() << "[" << i++ << "] " << child.second.data();
            csids_.emplace_back( csid );
        }
    }
}

const std::string&
ChemSpider::rid() const
{
    return rid_;
}

const std::vector< int >&
ChemSpider::csids() const
{
    return csids_;
}
    
