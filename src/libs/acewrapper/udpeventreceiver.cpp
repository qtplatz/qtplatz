/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include "udpeventreceiver.hpp"
#include <adportable/debug.hpp>
#include <workaround/boost/asio.hpp>
#include <boost/exception/all.hpp>
#include <boost/signals2.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <string>
#include <vector>

using boost::asio::ip::udp;

namespace acewrapper {

    enum { max_length = 1024 };

    class udpEventReceiver::impl {
    public:

        impl( boost::asio::io_service& io, short port ) : io_service_( io )
                                                        , sock_( io ) {
            open( port );
            do_receive();
        }

        ~impl() {
            if ( ! io_service_.stopped() ) {
                std::unique_lock< std::mutex > lock( mutex_ );
                sock_.cancel();
                cv_.wait( lock );
            }
            sock_.close();
        }

        void open( short port ) {
            auto endpoint = udp::endpoint( udp::v4(), port );
            sock_.open( endpoint.protocol() );
            sock_.set_option( udp::socket::reuse_address( true ) );
            sock_.bind( endpoint );
        }

        void  do_receive()  {
            sock_.async_receive_from(
                boost::asio::buffer(data_, max_length), sender_endpoint_,
                [this](boost::system::error_code ec, std::size_t bytes_recvd)  {
                    if (!ec && bytes_recvd > 0) {
                        signal_( data_, bytes_recvd, sender_endpoint_ );
                        do_send(bytes_recvd); // echo back
                    } else {
                        if ( ec == boost::system::errc::operation_canceled ) {
                            std::lock_guard< std::mutex > lock( mutex_ );
                            cv_.notify_one();
                        } else
                            do_receive();
                    }
                });
        }
        
        void do_send(std::size_t length)  {
            sock_.async_send_to(
                boost::asio::buffer(data_, length), sender_endpoint_,
                [this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {
                    do_receive();
                });
        }
        
        boost::asio::io_service& io_service_;
        boost::asio::ip::udp::socket sock_;
        boost::asio::ip::udp::endpoint sender_endpoint_;
        char data_[ max_length ];

        boost::signals2::signal< void(const char *, size_t, const boost::asio::ip::udp::endpoint& ) > signal_;
        std::mutex mutex_;
        std::condition_variable cv_;
    };
    
}

using namespace acewrapper;

udpEventReceiver::~udpEventReceiver()
{
    impl_.reset();
}

udpEventReceiver::udpEventReceiver( boost::asio::io_service& io, short port ) : impl_( new impl( io, port ) )
{
} 

void
udpEventReceiver::register_handler( std::function<void( const char *, size_t, const boost::asio::ip::udp::endpoint& )> h )
{
    impl_->signal_.connect( h );
}

boost::signals2::connection
udpEventReceiver::connect( std::function<void( const char *, size_t, const boost::asio::ip::udp::endpoint& )> h )
{
    return impl_->signal_.connect( h );
}

