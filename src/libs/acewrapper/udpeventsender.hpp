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

#include <boost/asio.hpp>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <functional>

namespace acewrapper {

    class udpEventSender {
    public:
        udpEventSender( boost::asio::io_service&, const char * host, const char * port );
        enum result_code { transaction_completed, transaction_timeout };
        
        bool send_to( const std::string&
                      , std::function< void( result_code, double, const char * ) > callback  = [](result_code, double, const char *){});

    private:
        enum { max_length = 1024 };
        boost::asio::ip::udp::socket sock_;
        boost::asio::ip::udp::endpoint endpoint_;
        char data_[ max_length ];
        std::condition_variable cv_;
        std::mutex mutex_;
    };

}
