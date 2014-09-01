/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#ifndef DGRAM_SERVER_HPP
#define DGRAM_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>

class dgram_server
{
public:
    dgram_server( boost::asio::io_service&, boost::asio::ip::udp::endpoint& remote, unsigned short rseq );
    void sendto( const char *, std::size_t );
    void conn_syn();

private:
    void start_receive();
    void handle_receive( const boost::system::error_code&, std::size_t );
    // void handle_send( boost::shared_ptr< std::string >, const boost::system::error_code&, std::size_t );
    void handle_send( boost::shared_array< char >, const boost::system::error_code&, std::size_t );
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remote_endpoint_;
    boost::array< char, 1500 > recv_buffer_;
    unsigned short remote_seq_;
    unsigned short local_seq_;
};

#endif // DGRAM_SERVER_HPP
