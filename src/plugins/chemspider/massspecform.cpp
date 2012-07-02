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

#include "massspecform.hpp"
#include "ui_massspecform.h"
#include <QtNetwork/QtNetwork>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#define _WIN32_WINNT 0x0700
#include <boost/asio.hpp>

MassSpecForm::MassSpecForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MassSpecForm)
{
    ui->setupUi(this);
	connect( ui->invoke_db, SIGNAL( clicked() ), this, SLOT( invoke_db_clicked() ) );
	connect( ui->invoke_mm, SIGNAL( clicked() ), this, SLOT( invoke_mm_clicked() ) );
}

MassSpecForm::~MassSpecForm()
{
    delete ui;
}

static const char * getdatabase = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?> \
<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"> \
  <soap:Body> \
    <GetDatabases xmlns=\"http://www.chemspider.com/\" /> \
  </soap:Body> \
</soap:Envelope>";

void
MassSpecForm::invoke_db_clicked()
{
	using boost::asio::ip::tcp;

	boost::asio::io_service io_service;
/*
	tcp::iostream stream( "www.chemspider.com", "http" );
	if ( stream ) {
		stream << "GET / HTTP/1.0\r\n\r\n";
		std::string text;
		std::getline( stream, text );
	}
*/
    // get a list of endpoints
	tcp::resolver resolver( io_service );
	tcp::resolver::query query( "www.chemspider.com", "http" );
	tcp::resolver::iterator endpoint_iterator = resolver.resolve( query );
    
    // try each endpoint
	tcp::socket socket( io_service );
	boost::asio::connect( socket, endpoint_iterator );
    
	boost::asio::streambuf request;
	std::ostream request_stream( &request );
#if 0
	request_stream << "GET / HTTP/1.0\r\n\r\n";
#endif
	request_stream << "POST /MassSpecAPI.asmx HTTP/1.1\r\n";
    request_stream << "Host: www.chemspider.com\r\n";
    request_stream << "Content-Type: text/xml; charset=utf-8\r\n";
	request_stream << "Content-Length: " << strlen( getdatabase ) << "\r\n";
    request_stream << "SOAPAction: \"http://www.chemspider.com/GetDatabases\"\r\n";
	request_stream << getdatabase << "\r\n\r\n";

	boost::asio::write( socket, request );

	boost::asio::streambuf response;

	try {
		boost::asio::read_until( socket, response, "\r\n" );
	} catch ( std::exception& ex ) {
		std::cout << ex.what();
	}

	std::istream response_stream( &response );
	std::string html;
	while ( response_stream ) {
		if ( std::getline( response_stream, html ) )
			ui->textBrowser->append( html.c_str() );
	} 
}

void
MassSpecForm::invoke_mm_clicked()
{
}
