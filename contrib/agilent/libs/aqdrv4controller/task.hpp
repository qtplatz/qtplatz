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
#include <acqrscontrols/acqiris_protocol.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind.hpp>
#include <chrono>
#include <cstdint>
#include <future>
#include <memory>
#include <vector>

namespace acqrscontrols {
    namespace aqdrv4 {
        class acqiris_client;
        class acqiris_method;
        class waveform;
        enum SubMethodType : unsigned int;
    }
    namespace ap240 {
        class method;
    }
}

namespace adacquire {
    class Receiver;
    class MasterObserver;
}

namespace aqdrv4controller {

    class WaveformObserver;

    class task {
        
        ~task();
        task();
        task( const task& ) = delete;
        const task& operator = ( const task& ) = delete;

    public:
        static task * instance();
        bool initialize();
        bool finalize();
        
        inline boost::asio::io_service& io_service() { return io_service_; }    
        inline boost::asio::io_service::strand& strand() { return strand_; }

        void prepare_for_run( std::shared_ptr< const acqrscontrols::ap240::method > );
#if 0
        // todo: method number issue need to be resolved
        void prepare_for_run( std::shared_ptr< const acqrscontrols::aqdrv4::acqiris_method >, acqrscontrols::aqdrv4::SubMethodType );
#endif
        void event_out( uint32_t );

        void push( std::shared_ptr< acqrscontrols::aqdrv4::waveform > );
        void push( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > );

        ///
        void connect( const std::string& server, const std::string& port );

        void connect_client( std::shared_ptr< adacquire::Receiver >, const std::string& );
        void disconnect_client( std::shared_ptr< adacquire::Receiver > );

        inline adacquire::MasterObserver * masterObserver() { return masterObserver_.get(); }
        
    private:
        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
        boost::asio::io_service::strand strand_;
        std::mutex mutex_;

        std::vector< std::thread > threads_;
        std::unique_ptr< acqrscontrols::aqdrv4::acqiris_client > client_;

        typedef std::pair< std::shared_ptr< adacquire::Receiver >, std::string > receiver_pair_t;
        std::vector< receiver_pair_t > receivers_;
        std::shared_ptr< adacquire::MasterObserver > masterObserver_;
        std::shared_ptr< WaveformObserver > waveformObserver_;
        void reply_message( int, int );
    };

}

