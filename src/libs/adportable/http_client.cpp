// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "http_client.hpp"

using boost::asio::ip::tcp;

using namespace adportable;

http_client::~http_client()
{
}

http_client::http_client( boost::asio::io_service& io_service
                          , const std::string& server ) : io_service_( io_service )
                                                        , server_( server )
{
    tcp::resolver resolver( io_service );
    tcp::resolver::query query( server, "http" );
    endpoint_iterator_ = resolver.resolve(query);
}

bool
http_client::sync_write( boost::asio::streambuf& request
                         , unsigned int& status_code
                         , std::string& http_version
                         , std::ostream& result )
{
    try {
        tcp::socket socket( io_service_ );
        boost::asio::connect( socket, endpoint_iterator_ );

        // Send the request.
        boost::asio::write( socket, request );

        // Read the response status line. The response streambuf will automatically
        // grow to accommodate the entire line. The growth may be limited by passing
        // a maximum size to the streambuf constructor.
        boost::asio::streambuf response;
        boost::asio::read_until( socket, response, "\r\n" );

        // Check that response is OK.
        std::istream response_stream( &response );
        response_stream >> http_version;
        response_stream >> status_code;
        
        std::string status_message;
        std::getline( response_stream, status_message );
        if ( !response_stream || http_version.substr(0, 5) != "HTTP/") {
            result << "Invalid response\n";
            return false;
        }

        if ( status_code != 200 ) {
            result << "Response returned with status code " << status_code << "\n";
            return false;
        }
        
        // Read the response headers, which are terminated by a blank line.
        boost::asio::read_until(socket, response, "\r\n\r\n");

        // Process the response headers.
        std::string header;
        while ( std::getline( response_stream, header ) && header != "\r" )
            result << header << "\n";
        result << "\n";

        // Write whatever content we already have to output.
        if (response.size() > 0)
            result << &response;
        
        // Read until EOF, writing data to output as we go.
        boost::system::error_code error;
        while (boost::asio::read(socket, response,
                                 boost::asio::transfer_at_least(1), error))
            result << &response;
        
        if (error != boost::asio::error::eof)
            throw boost::system::system_error(error);
        
    } catch ( std::exception& e ) {
        result << "Exception: " << e.what() << "\n";
        return false;
    }
    
    return true;
}

bool
http_client::get( const std::string& path, std::ostream& result, unsigned int& status_code )
{
    try {
        boost::asio::streambuf request;
        std::ostream request_stream( &request );

        request_stream << "GET " << path << " HTTP/1.0\r\n";
        request_stream << "Host: " << server_ << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";

        std::string http_version;
        return sync_write( request, status_code, http_version, result );
        
    } catch ( std::exception& e ) {
        result << "Exception: " << e.what() << "\n";
    }
    return false;
}

bool
http_client::post( const std::string& path
                   , std::ostream& result
                   , unsigned int& status_code
                   , const char * content_type )
{
    result << "Not implemented yet";
    return false;
}
