/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include <functional>
#include <memory>
#include <boost/signals2.hpp>
#include <workaround/boost/asio.hpp>

namespace acewrapper {

    class udpEventReceiver {
    public:
        ~udpEventReceiver();

        udpEventReceiver( boost::asio::io_service&, short port = 7125 );
        
        void close();

        void register_handler( std::function<void( const char *, size_t, const boost::asio::ip::udp::endpoint& )> );

        boost::signals2::connection connect( std::function<void( const char *, size_t, const boost::asio::ip::udp::endpoint& )> );

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}


