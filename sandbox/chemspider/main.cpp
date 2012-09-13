/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include <QCoreApplication>

#include <iostream>
#include <fstream>
#include <string>
#include <boost/asio.hpp>

static const char * getdatabase = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?> \
<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"> \
  <soap:Body> \
    <GetDatabases xmlns=\"http://www.chemspider.com/\" /> \
  </soap:Body> \
</soap:Envelope>";

static const char * getdatabase2 = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?> \
<soap12:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" \
                 xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" \
                 xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\"> \
  <soap12:Body> \
    <GetDatabases xmlns=\"http://www.chemspider.com/\" /> \
  </soap12:Body> \
</soap12:Envelope>";

int
main(int argc, char *argv[])
{
	using boost::asio::ip::tcp;

	do { 
		std::ofstream of( "query.xml" );
		of << getdatabase2 << std::endl;
	} while(0);

	boost::asio::io_service io_service;
	tcp::resolver resolver( io_service );
	tcp::resolver::query query( "www.chemspider.com", "http" );
	tcp::resolver::iterator endpoint_iterator = resolver.resolve( query );

	tcp::socket socket( io_service );
	boost::asio::connect( socket, endpoint_iterator );

	boost::asio::streambuf request;
	std::ostream request_stream( &request );

	request_stream << "POST /MassSpecAPI.asmx HTTP/1.1\r\n";
	request_stream << "Host: www.chemspider.com\r\n";
	request_stream << "Content-Type: application/soap+xml; charset=utf-8\r\n";
	request_stream << "Content-Length: " << strlen( getdatabase2 ) << "\r\n\r\n";
	request_stream << getdatabase2 << "\r\n\r\n" << std::flush;

    /*
	request_stream << "POST /MassSpecAPI.asmx HTTP/1.1\r\n";
    request_stream << "Host: www.chemspider.com\r\n";
    request_stream << "Content-Type: text/xml; charset=utf-8\r\n";
	request_stream << "Content-Length: " << strlen( getdatabase ) << "\r\n";
    request_stream << "SOAPAction: \"http://www.chemspider.com/GetDatabases\"\r\n";
	request_stream << getdatabase << "\r\n\r\n";
	*/

	boost::asio::write( socket, request );
	boost::asio::streambuf response;
	boost::system::error_code error;

	try {
		boost::asio::read_until( socket, response, "\r\n" );
	} catch ( std::exception& ex ) {
		std::cout << ex.what();
	}

	std::istream response_stream( &response );
	std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
	std::string status_message;
	std::getline( response_stream, status_message );

	if ( !response_stream || http_version.substr(0, 5) != "HTTP/" )
		std::cout << "Invalid response\n";

	if ( status_code != 200 )
		std::cout << "Response returned with status code " << status_code << std::endl;

	std::cout << http_version << " " << status_code << " " << status_message << std::endl;

	try {
		boost::asio::read_until( socket, response, "\r\n\r\n" );
	} catch ( std::exception& ex ) {
		std::cout << ex.what();
	}

	std::string line;
	while ( std::getline( response_stream, line ) ) {
		if ( line == "\n" || line == "\r" )
			break;
		std::cout << line << std::endl;
	}

	do {
		std::ofstream of( "response.xml" );
		of << &response;

		while ( boost::asio::read( socket, response, boost::asio::transfer_at_least(1), error ) )
            of << &response;

	} while ( 0 );

}
