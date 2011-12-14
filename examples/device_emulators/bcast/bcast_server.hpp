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

#ifndef BCAST_SERVER_HPP
#define BCAST_SERVER_HPP

#include <boost/asio.hpp>
#include "lifecycle.hpp"

class bcast_server {
public:
    bcast_server( boost::asio::io_service&, int port );
    boost::asio::ip::udp::socket& socket() { return socket_; }

protected:

private:
    void start_receive();
    void handle_receive( const boost::system::error_code&, std::size_t );
    void handle_send( boost::shared_ptr< std::string >, const boost::system::error_code&, std::size_t );
    void handle_send_to( const boost::system::error_code& );
    void handle_timeout( const boost::system::error_code& );
    boost::asio::ip::udp::endpoint remote_endpoint_;
    boost::array< char, 1024 > recv_buffer_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::deadline_timer timer_;
    unsigned short local_seq_;
    unsigned short remote_seq_;
};

#endif // BCAST_SERVER_HPP
