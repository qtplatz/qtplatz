/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "eventbroker.h"
#include <acewrapper/ifconfig.hpp>
#include <boost/asio.hpp>
#include <atomic>
#include <mutex>
#include <memory>
#include <thread>

namespace acewrapper { class udpEventSender; }


namespace eventbroker {

    enum event_id {
        event_inject    = 0x00000001
        , event_warning = 0x40000000
        , event_error   = 0x80000000
    };

    class document {
        document();
        static std::atomic< document * > instance_;
        static std::mutex mutex_;

        std::vector< event_handler > handlers_;
        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
        std::unique_ptr< acewrapper::udpEventSender > udpSender_;
        std::vector< std::thread > threads_;

    public:
        ~document();
        static document * instance();
        bool register_handler( event_handler );
        bool unregister_handler( event_handler );
        bool bind( const char * host, const char * port, bool bcast );
        bool event_out( uint32_t );
        bool event_out( event_id id, uint64_t epoch_time, const std::string& json = "{}" );
    };

}
