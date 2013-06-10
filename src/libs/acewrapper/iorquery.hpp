/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#ifndef IORQUERY_HPP
#define IORQUERY_HPP

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/function.hpp>
#if defined _MSC_VER || defined __APPLE__ // assume -stdlib=libc++
# include <cstdint>
#else
# include <tr1/cstdint>
#endif

namespace acewrapper {

    template<class T> struct ior_query_reply {};

    class iorQuery : boost::noncopyable {
    public:
        iorQuery( boost::asio::io_service& io_service
                  , boost::function<void (const std::string&, const std::string&)> );
        bool open();
        void close();
        void suspend();
        void resume();
    private:
        void send_query();
        void handle_timeout( const boost::system::error_code& );
        void handle_receive( const boost::system::error_code&, std::size_t );
        void initiate_timer();
        void start_receive();
        boost::asio::io_service& io_service_;
        boost::function<void (const std::string&, const std::string&)> handle_reply_;
        boost::asio::ip::udp::socket socket_;
        boost::asio::ip::udp::endpoint endpoint_;
        boost::array< char, 1500u > recv_buffer_;
        boost::asio::deadline_timer timer_;
        std::size_t interval_;
        bool suspend_;
    };

}

#endif // IORQUERY_HPP
