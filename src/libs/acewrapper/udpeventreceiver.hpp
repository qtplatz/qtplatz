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

#pragma once

#include <workaround/boost/asio.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace acewrapper {

    class udpEventReceiver {
    public:
        ~udpEventReceiver();
        udpEventReceiver( boost::asio::io_service&, short port = 7125 );
        
        bool open( unsigned short port = 0 );
        void close();

        void do_receive();
        void do_send( size_t );
        void register_handler( std::function<void( const char *, size_t, const boost::asio::ip::udp::endpoint& )> );
    private:
        enum { max_length = 1024 };
        boost::asio::io_service& io_service_;
        boost::asio::ip::udp::socket sock_;
        boost::asio::ip::udp::endpoint sender_endpoint_;
        char data_[ max_length ];
        std::function<void( const char *, size_t, const boost::asio::ip::udp::endpoint& )> handler_;
        std::mutex mutex_;
        std::condition_variable cv_;
    };

}


