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

//#include <QtCore/QCoreApplication>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "mcast_sender.hpp"
#include "dgram_server.hpp"
#include "../hello/lifecycle.hpp"

using boost::asio::ip::udp;

class dgram_state_machine {
    // this class handling port 7000, a.k.a. well known port for 
    // defice discovery & life cycle protocol
public:
    dgram_state_machine( boost::asio::io_service& io ) : io_service_( io )
                                                       , socket_( io, udp::endpoint( udp::v4(), 7000 ) ) {
        start_receive();
    }

    void start_receive() {
        socket_.async_receive_from( boost::asio::buffer( recv_buffer_ )
                                    , remote_endpoint_
                                    , boost::bind( &dgram_state_machine::handle_receive
                                                   , this
                                                   , boost::asio::placeholders::error
                                                   , boost::asio::placeholders::bytes_transferred ) );
    }
    
    void handle_receive( const boost::system::error_code& error, std::size_t len ) {

        if ( ! error || error == boost::asio::error::message_size ) {
            
            std::cout << "dgram_state_machine receive from: " 
                      << remote_endpoint_.address().to_string() 
                      << "/" << remote_endpoint_.port()
                      << std::endl;
            
            const LifeCycleFrame * pf = reinterpret_cast<const LifeCycleFrame *>( &recv_buffer_[0] );
            
            if ( len >= sizeof( LifeCycleFrame ) &&
                 ( pf->endian_mark == 0xfffe || pf->endian_mark == 0xfeff ) &&
                 ( pf->proto_version == 0x0001 ) && 
                 ( pf->ctrl == 0 ) ) { // ctrl

                boost::uint32_t command = 
                    *reinterpret_cast<const boost::uint32_t *>( &recv_buffer_[ pf->hoffset ] );
                
                boost::uint16_t remote_seq = *reinterpret_cast< uint16_t *>( &recv_buffer_[ sizeof(LifeCycleFrame) ] );
                
                if ( command == CONN_SYN ) {
                    // if not already connected for this 'remote' port
                    // todo: this implementation is not correct. port# should be compared with ipaddr
                    
                    if ( sessions_.find( remote_endpoint_.port() ) == sessions_.end() ) {
                        // create new session, and reply CONN_SYN_ACK with new local port#
                        std::cout << "CONN_SYN accepted" << std::endl;
                        sessions_[ remote_endpoint_.port() ].reset( 
                            new dgram_server( io_service_, remote_endpoint_, remote_seq ) );
                    }
                    sessions_[ remote_endpoint_.port() ]->conn_syn();
                }
            }
        }
        // continue next data
        start_receive();
    }
private:
    boost::asio::io_service& io_service_;
    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    boost::array< char, 1500 > recv_buffer_;
    std::map< unsigned short, boost::shared_ptr< dgram_server > > sessions_;
};

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    using boost::asio::ip::udp;

    boost::asio::io_service io_service;

    mcast_sender s( io_service, boost::asio::ip::address::from_string( "224.9.9.2" ) );
    dgram_state_machine hello( io_service );

    io_service.run();

    return 0;
}
