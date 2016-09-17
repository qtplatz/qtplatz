/**************************************************************************
** Copyright (C) 2014-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include "semaphore.hpp"
#include "acqiris_protocol.hpp"
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <cstdint>
#include <future>
#include <memory>
#include <vector>

namespace aqdrv4 {

    class acqiris_method;
    class waveform;
    
    namespace client {

        class tcp_task {

            ~tcp_task();
            tcp_task();
            tcp_task( const tcp_task& ) = delete;
            const tcp_task& operator = ( const tcp_task& ) = delete;
        public:
            static tcp_task * instance();
            bool initialize();
            bool finalize();
            
            inline boost::asio::io_service& io_service() { return io_service_; }    
            inline boost::asio::io_service::strand& strand() { return strand_; }
            
            void prepare_for_run( std::shared_ptr< const aqdrv4::acqiris_method > );

            void push( std::shared_ptr< aqdrv4::waveform > );
            void push( std::shared_ptr< aqdrv4::acqiris_method > );
            
        private:
            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;
            boost::asio::io_service::strand strand_;
            boost::asio::steady_timer timer_;
            std::atomic< bool > worker_stopping_;
            adportable::semaphore sema_;
            std::vector< std::thread > threads_;
            std::atomic_flag acquire_posted_;
            std::chrono::time_point<std::chrono::system_clock> tp_data_handled_;
            
            //void acquire( digitizer * );
            void worker_thread();
            void handle_timer( const boost::system::error_code& ec );
        };

    }
}
