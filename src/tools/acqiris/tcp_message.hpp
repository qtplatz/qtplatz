/**************************************************************************
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this 
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

#include <memory>
#include <boost/asio.hpp>

class tcp_message {
public:
    enum { header_length = 4 };
    enum { max_payload_length = 1024 * 8 };

    tcp_message() : payload_length_ ( 0 ) {
    }

    const char * data() const {
        return data_;
    }

    tcp_server( boost::asio::io_service& io, short port )
        : acceptor_( io, boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), port ) )
        , socket_( io ) {
        do_accept();
    }
    inline const boost::asio::ip::tcp::acceptor& acceptor() const { return acceptor_; }
    
private:
    void do_accept() {
        acceptor_.async_accept( socket_, [this](boost::system::error_code ec ) {
                if ( !ec ) {
                    std::make_shared< session >( std::move( socket_ ) )->start();
                }
                do_accept();
            });
    }

    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
};

