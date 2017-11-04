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

namespace chemistry {

    // reference: http://www.chemspider.com/search.asmx
    //            http://www.chemspider.com/synonyms.asmx <-- this returns 'no access permission error'

    static const char * logfile = "csdebug.txt";
    
    namespace chemspider {

        struct soap_envelope {

            static void set_xml_attrb( boost::property_tree::ptree& envelope )  {
                envelope.put( "<xmlattr>.xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
                envelope.put( "<xmlattr>.xmlns:xsd", "http://www.w3.org/2001/XMLSchema" );
                envelope.put( "<xmlattr>.xmlns:soap12", "http://www.w3.org/2003/05/soap-envelope" );
            }

            static void set_xml_xmlns( boost::property_tree::ptree& envelope, const std::string& key )  {
                auto& node = envelope.get_child( key );
                node.put( "<xmlattr>.xmlns", "http://www.chemspider.com/" );
            }

            static std::string toXml( const boost::property_tree::ptree& pt ) {
                using namespace boost::property_tree;
                std::ostringstream xml;
                xml_parser::write_xml( xml, pt, boost::property_tree::xml_writer_make_settings<std::string>( ' ', 2 ) );
                return xml.str();
            }
        };

        // Search
        template< typename T = soap_envelope >
        struct soap_search_request {
            bool operator()( std::ostream& request_stream, const boost::property_tree::ptree& pt ) const {
                auto xml = T::toXml( pt );
                request_stream << "POST /search.asmx HTTP/1.1\r\n";
                request_stream << "Host: www.chemspider.com\r\n";
                request_stream << "Content-Type: application/soap+xml; charset=utf-8\r\n";
                request_stream << "Content-Length: " << xml.length() << "\r\n";
                request_stream << "Connection: close\r\n\r\n";
                request_stream << xml;
                return true;
            }
        };

        // Synonyms
        template< typename T = soap_envelope >
        struct soap_synonyms_request {
            bool operator()( std::ostream& request_stream, const boost::property_tree::ptree& pt ) const {
                auto xml = T::toXml( pt );
                request_stream << "POST /Synonyms.asmx HTTP/1.1\r\n";
                request_stream << "Host: www.chemspider.com\r\n";
                request_stream << "Content-Type: application/soap+xml; charset=utf-8\r\n";
                request_stream << "Content-Length: " << xml.length() << "\r\n";
                request_stream << "Connection: close\r\n\r\n";
                request_stream << xml;
                return true;
            }
        };
        
        struct soap_response {

            bool operator()( adurl::client& cs, boost::property_tree::ptree& pt ) {
                std::istream response_stream( &cs.response() );
                boost::property_tree::xml_parser::read_xml( response_stream, pt );
                return true;
            }
        };

        template< typename T > struct soap_response_data {
            boost::optional< const boost::property_tree::ptree& > operator ()( const boost::property_tree::ptree& pt ) const {
                return pt.get_child_optional( ( boost::format( "soap:Envelope.soap:Body.%1%" ) % T::rkey ).str() );
            }
        };

        struct AsyncSimpleSearch {
            static constexpr const char * key = "AsyncSimpleSearch";
            static constexpr const char * rkey = "AsyncSimpleSearchResult";
            void operator()( boost::property_tree::ptree& pt, const std::string& stmt, const std::string& token ) const {
                // SOAP 1.2
                pt.add( "soap12:Envelope.soap12:Body.AsyncSimpleSearch.query", stmt );
                pt.add( "soap12:Envelope.soap12:Body.AsyncSimpleSearch.token", token );
                auto& envelope = pt.get_child( "soap12:Envelope" );
                soap_envelope::set_xml_attrb( envelope );
                soap_envelope::set_xml_xmlns( envelope, ( boost::format( "soap12:Body.%1%" ) % key ).str() );
            }
        };

        struct GetAsyncSearchResult {
            static constexpr const char * key = "GetAsyncSearchResult";
            static constexpr const char * rkey = "GetAsyncSearchResultResponse.GetAsyncSearchResultResult";
            const boost::property_tree::ptree&
            operator()( boost::property_tree::ptree& pt, const std::string& rid, const std::string& token ) const {
                // SOAP 1.2
                pt.add( "soap12:Envelope.soap12:Body.GetAsyncSearchResult.rid", rid );
                pt.add( "soap12:Envelope.soap12:Body.GetAsyncSearchResult.token", token );
                auto& envelope = pt.get_child( "soap12:Envelope" );
                soap_envelope::set_xml_attrb( envelope );
                soap_envelope::set_xml_xmlns( envelope, ( boost::format( "soap12:Body.%1%" ) % key ).str() );
                return pt;
            }
        };

        struct GetAsyncSearchStatus {
            static constexpr const char * key = "GetAsyncSearchStatus";
            static constexpr const char * rkey = "GetAsyncSearchStatusResponse";
            const boost::property_tree::ptree&
            operator()( boost::property_tree::ptree& pt, const std::string& rid, const std::string& token ) const {
                // SOAP 1.2
                pt.add( ( boost::format( "soap12:Envelope.soap12:Body.%1%.rid" ) % key ).str(), rid );
                pt.add( ( boost::format( "soap12:Envelope.soap12:Body.%1%.token" ) % key ).str(), token );
                auto& envelope = pt.get_child( "soap12:Envelope" );
                soap_envelope::set_xml_attrb( envelope );
                soap_envelope::set_xml_xmlns( envelope, ( boost::format( "soap12:Body.%1%" ) % key ).str() );
                return pt;
            }
        };
        
        struct GetCompoundInfo {
            static constexpr const char * key = "GetCompoundInfo";
            static constexpr const char * rkey = "GetCompoundInfoResponse.GetCompoundInfoResult";
            const boost::property_tree::ptree&
            operator()( boost::property_tree::ptree& pt, int csid, const std::string& token ) const {
                // SOAP 1.2
                pt.add( ( boost::format( "soap12:Envelope.soap12:Body.%1%.CSID" ) % key ).str(), csid );
                pt.add( ( boost::format( "soap12:Envelope.soap12:Body.%1%.token" ) % key ).str(), token );
                auto& envelope = pt.get_child( "soap12:Envelope" );
                soap_envelope::set_xml_attrb( envelope );
                soap_envelope::set_xml_xmlns( envelope, ( boost::format( "soap12:Body.%1%" ) % key ).str() );
                return pt;
            }
        };

        struct CSID2ExtRefs {
            static constexpr const char * key = "CSID2ExtRefs";
            static constexpr const char * rkey = "CSID2ExtRefsResponse";
            const boost::property_tree::ptree&
            operator()( boost::property_tree::ptree& pt, int csid, const std::vector< std::string >& datasources, const std::string& token ) const {
                // SOAP 1.2
                pt.add( ( boost::format( "soap12:Envelope.soap12:Body.%1%.CSID" ) % key ).str(), csid );

                for ( auto& ds : datasources ) 
                    pt.add( ( boost::format( "soap12:Envelope.soap12:Body.%1%.datasources.string" ) % key ).str(), ds );

                pt.add( ( boost::format( "soap12:Envelope.soap12:Body.%1%.token" ) % key ).str(), token );
                auto& envelope = pt.get_child( "soap12:Envelope" );
                soap_envelope::set_xml_attrb( envelope );
                soap_envelope::set_xml_xmlns( envelope, ( boost::format( "soap12:Body.%1%" ) % key ).str() );
                return pt;
            }
        };

        // /synonyms
        struct RetrieveByCSIDList {
            static constexpr const char * key = "RetrieveByCSIDList";
            static constexpr const char * rkey = "RetrieveByCSIDListResponse";
            const boost::property_tree::ptree&
            operator()( boost::property_tree::ptree& pt, const std::vector< int >& csids, const std::string& token ) const {
                // SOAP 1.2
                for ( auto csid: csids )
                    pt.add( ( boost::format( "soap12:Envelope.soap12:Body.%1%.csids.int" ) % key ).str(), csid );

                pt.add( ( boost::format( "soap12:Envelope.soap12:Body.%1%.token" ) % key ).str(), token );
                auto& envelope = pt.get_child( "soap12:Envelope" );
                soap_envelope::set_xml_attrb( envelope );
                soap_envelope::set_xml_xmlns( envelope, ( boost::format( "soap12:Body.%1%" ) % key ).str() );
                return pt;
            }
        };

        struct GetStructureSynonyms {
            static constexpr const char * key = "GetStructureSynonyms";
            static constexpr const char * rkey = "GetStructureSynonymsResponse";
            const boost::property_tree::ptree&
            operator()( boost::property_tree::ptree& pt, const std::string& mol, const std::string& token ) const {
                // SOAP 1.2
                pt.add( ( boost::format( "soap12:Envelope.soap12:Body.%1%.mol" ) % key ).str(), mol );
                pt.add( ( boost::format( "soap12:Envelope.soap12:Body.%1%.token" ) % key ).str(), token );
                auto& envelope = pt.get_child( "soap12:Envelope" );
                soap_envelope::set_xml_attrb( envelope );
                soap_envelope::set_xml_xmlns( envelope, ( boost::format( "soap12:Body.%1%" ) % key ).str() );
                return pt;
            }
        };
        

        constexpr const char * AsyncSimpleSearch::key;
        constexpr const char * AsyncSimpleSearch::rkey;
        constexpr const char * GetAsyncSearchStatus::key;
        constexpr const char * GetAsyncSearchStatus::rkey;        
        constexpr const char * GetAsyncSearchResult::key;
        constexpr const char * GetAsyncSearchResult::rkey;
        constexpr const char * GetCompoundInfo::key;
        constexpr const char * GetCompoundInfo::rkey;
        constexpr const char * CSID2ExtRefs::key;
        constexpr const char * CSID2ExtRefs::rkey;
        constexpr const char * RetrieveByCSIDList::key;
        constexpr const char * RetrieveByCSIDList::rkey;
        constexpr const char * GetStructureSynonyms::key;
        constexpr const char * GetStructureSynonyms::rkey;
        
        
    } // namespace chemspider
    
    static void print( std::ostream& outf, const boost::property_tree::ptree& pt )
    {
        using boost::property_tree::ptree;
        
        ptree::const_iterator end = pt.end();
        for (ptree::const_iterator it = pt.begin(); it != end; ++it) {
            outf << it->first << ": " << it->second.get_value<std::string>() << std::endl;
            print( outf, it->second );
        }    
    }
    
    static void debug_response( const boost::property_tree::ptree& pt )
    {
        using namespace boost::property_tree;
        std::ofstream of( logfile, std::ios_base::out | std::ios_base::app );
        xml_parser::write_xml( of, pt, xml_writer_make_settings<std::string>( ' ', 2 ) );
        of << std::endl;
    }
}

using namespace chemistry;


ChemSpider::ChemSpider( const std::string& token ) : token_( token )
{
}

ChemSpider::~ChemSpider()
{
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
    
bool
ChemSpider::AsyncSimpleSearch( const std::string& stmt )
{
    auto request = std::make_unique< boost::asio::streambuf >();
    std::ostream request_stream ( request.get() );

    do {
        boost::property_tree::ptree pt;
        chemspider::AsyncSimpleSearch()( pt, stmt, token_ );
        chemspider::soap_search_request< chemspider::soap_envelope >()( request_stream, pt );
    } while ( 0 );

    boost::asio::io_service io_service;

    adurl::client cs( io_service, std::move( request ), "www.chemspider.com" );

    io_service.run();

    if ( cs.status_code() != 200 ) {

        ADDEBUG() << &cs.response_header();
        ADDEBUG() << "status_code: " << cs.status_code() << ", " << cs.status_message();

    } else {
        boost::property_tree::ptree pt;
        if ( chemspider::soap_response()( cs, pt ) ) {
            // ChemSpider always return SOAP 1.1 format
            if ( auto resp = pt.get_child_optional( "soap:Envelope.soap:Body.AsyncSimpleSearchResponse.AsyncSimpleSearchResult" ) ) {
                rid_ = resp.get().data();
                return true;
                // return getAsyncSearchResult( rid_ );
            } else {
                ADDEBUG() << "AsyncSimpleSearchResult node not found";
            }
        }
    }
    return false;
}

bool
ChemSpider::GetAsyncSearchStatus( std::string& response )
{
    auto request = std::make_unique< boost::asio::streambuf >();
    std::ostream request_stream ( request.get() );

    response = std::string();

    do {
        boost::property_tree::ptree pt;
        chemspider::GetAsyncSearchStatus()( pt, rid_, token_ );
        chemspider::soap_search_request< chemspider::soap_envelope >()( request_stream, pt );
    } while ( 0 );

    boost::asio::io_service io_service;

    adurl::client cs( io_service, std::move( request ), "www.chemspider.com" );

    io_service.run();

    if ( cs.status_code() != 200 ) {

        ADDEBUG() << &cs.response_header();
        ADDEBUG() << "status_code: " << cs.status_code() << ", " << cs.status_message();

    } else {

        boost::property_tree::ptree pt;
        if ( chemspider::soap_response()( cs, pt ) ) {

            debug_response( pt );

            if ( auto resp = chemspider::soap_response_data< chemspider::GetAsyncSearchStatus >()( pt ) ) {
                if ( auto _status = resp.get().get_optional< std::string >( "GetAsyncSearchStatusResult" ) ) {
                    response = _status.get();
                    return response == "ResultReady";
                }
            }
        }
    }
    return false;
}

bool
ChemSpider::GetAsyncSearchResult()
{
    return getAsyncSearchResult( rid_ );
}

bool
ChemSpider::getAsyncSearchResult( const std::string& rid )
{
    auto request = std::make_unique< boost::asio::streambuf >();
    std::ostream request_stream ( request.get() );

    do {
        boost::property_tree::ptree pt;
        chemspider::soap_search_request< chemspider::soap_envelope >()( request_stream, chemspider::GetAsyncSearchResult()( pt, rid, token_ ) );
    } while(0);
    
    boost::asio::io_service io_service;
    adurl::client cs( io_service, std::move( request ), "www.chemspider.com"  );

    io_service.run();

    if ( cs.status_code() != 200 ) {
        ADDEBUG() << &cs.response_header();
        ADDEBUG() << "status_code: " << cs.status_code() << ", " << cs.status_message();
        return false;
    } else {
        boost::property_tree::ptree pt;
        if ( chemspider::soap_response()( cs, pt ) ) {

            debug_response( pt );

            if ( auto resp = chemspider::soap_response_data< chemspider::GetAsyncSearchResult >()( pt ) ) {
                for( auto& child : resp.get() ) {
                    try {
                        int csid = boost::lexical_cast< int >( child.second.data() );
                        if ( std::find( csids_.begin(), csids_.end(), csid ) == csids_.end() )
                            csids_.emplace_back( csid );
                    } catch ( boost::bad_lexical_cast& ex ) {
                        ADDEBUG() << ex.what();
                    }
                }
            }
        }
        return true;
    }
}

bool
ChemSpider::GetCompoundInfo( int csid, std::string& smiles, std::string& InChI, std::string& InChIKey )
{
    auto request = std::make_unique< boost::asio::streambuf >();
    std::ostream request_stream ( request.get() );

    do {
        boost::property_tree::ptree pt;
        chemspider::soap_search_request< chemspider::soap_envelope >()( request_stream, chemspider::GetCompoundInfo()( pt, csid, token_ ) );
    } while(0);
    
    boost::asio::io_service io_service;
    adurl::client cs( io_service, std::move( request ), "www.chemspider.com" );

    io_service.run();

    if ( cs.status_code() != 200 ) {
        ADDEBUG() << &cs.response_header();
        ADDEBUG() << "status_code: " << cs.status_code() << ", " << cs.status_message();
        return false;
    } else {
        boost::property_tree::ptree pt;
        if ( chemspider::soap_response()( cs, pt ) ) {

            debug_response( pt );
            
            if ( auto resp = chemspider::soap_response_data< chemspider::GetCompoundInfo >()( pt ) ) {

                // std::ofstream of( "compoundinfo.txt", std::ios_base::out );
                // print( of, resp.get() );
                
                if ( auto _csid = resp.get().get_optional< int >( "CSID" ) ) {
                    if ( _csid.get() == csid ) {
                        if ( auto _smiles = resp.get().get_optional< std::string >( "SMILES" ) ) {
                            smiles = _smiles.get();
                            smiles_[ csid ] = smiles;
                        }
                        if ( auto _InChI = resp.get().get_optional< std::string >( "InChI" ) ) {
                            InChI = _InChI.get();
                            inchis_[ csid ] = InChI;
                        }
                        if ( auto _InChIKey = resp.get().get_optional< std::string >( "InChIKey" ) ) {
                            InChIKey = _InChIKey.get();
                        }
                    }
                }
            }
            return true;            
        }
    }
    return false;
}

bool
ChemSpider::GetSynonyms( int csid, std::vector< std::string >& synonyms )
{
    synonyms.clear();

    auto request = std::make_unique< boost::asio::streambuf >();
    std::ostream request_stream ( request.get() );

    do {
        // xml format was verified, though this returns 'internal server error' -- it seems no access permission.
        boost::property_tree::ptree pt;
        //chemspider::soap_synonyms_request< chemspider::soap_envelope >()( request_stream, chemspider::RetrieveByCSIDList()( pt, csids_, token_ ) );
        chemspider::soap_synonyms_request< chemspider::soap_envelope >()( request_stream, chemspider::GetStructureSynonyms()( pt, inchis_[ csid ], token_ ) );
    } while(0);
    
    boost::asio::io_service io_service;
    adurl::client cs( io_service, std::move( request ), "www.chemspider.com" );

    io_service.run();

    if ( cs.status_code() != 200 ) {

        ADDEBUG() << &cs.response_header();
        ADDEBUG() << "status_code: " << cs.status_code() << ", " << cs.status_message();
        return false;

    } else {
        boost::property_tree::ptree pt;
        if ( chemspider::soap_response()( cs, pt ) ) {

            debug_response( pt );
            
            return true;            
        }
    }
    return false;
}
