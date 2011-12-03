/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#include <QtCore/QCoreApplication>
#include <boost/asio.hpp>
#include <boost/array.hpp>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    using boost::asio::ip::udp;
    
    boost::asio::io_service io_service;
    
    //udb::resolver resolver( io_service );
    //udp::resolver::query query( udp::v4(), "localhost", "hello" );
    udp::endpoint receiver_endpoint( udp::endpoint( udp::v4(), 7000 ) );
    
    udp::socket socket( io_service );
    socket.open( udp::v4() );

    boost::array< char, 1 > send_buf = {{ 0 }};
    socket.send_to( boost::asio::buffer( send_buf ), receiver_endpoint );
    
    boost::array< char, 128 > recv_buf;
    udp::endpoint sender_endpoint;
    size_t len = socket.receive_from( boost::asio::buffer( recv_buf ), sender_endpoint );
    
    std::cout.write( recv_buf.data(), len );

    std::cout << "received from: " 
              << sender_endpoint.address().to_string() 
              << "/" << sender_endpoint.port()
              << std::endl;
}
