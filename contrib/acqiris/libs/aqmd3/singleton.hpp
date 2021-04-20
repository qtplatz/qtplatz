/**************************************************************************
** Copyright (C) 2018-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <boost/asio.hpp>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <ostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace adacquire {
    class Receiver;
    class MasterObserver;
    namespace SignalObserver {
        class Observer;
    }
}

namespace adportable {
    class semaphore;
}

namespace aqmd3controls {
    class waveform;
    class method;
}

namespace aqmd3 {

    //class udp_client;
    //class message_serializer;
    class dataframe;
    class waveform;
    class session;
    class WaveformObserver;
    class meta_data;
    class pkdObserver;
    class digitizer;

    class singleton {
        singleton();
        singleton( const singleton& ) = delete;
        singleton& operator = ( singleton& ) = delete;
    public:
        ~singleton();
        static singleton * instance();

        inline boost::asio::io_service& io_service() { return io_service_; }

        bool initialize();
        bool finalize();

        bool is_open() const;

        //void send( std::shared_ptr< message_serializer > ) const;
        //const boost::asio::ip::udp::endpoint& endpoint() const;

        uint16_t get_sequence_number();
        void enqueue( std::string&& );
        bool dequeue( std::string& );
        bool dequeue( std::string&, const std::chrono::milliseconds& duration );
        void reply_message( int msg, int value );

        bool connect( std::shared_ptr< adacquire::Receiver > ptr, const std::string& token );
        bool disconnect( std::shared_ptr< adacquire::Receiver > ptr );

        adacquire::SignalObserver::Observer * getObserver();

        void set_inject_trigger_in( bool set = true );
        bool inject_trigger() const;

        void set_refresh_histogram( bool );
        void set_refresh_period( int );
        void set_hvdg_status( std::string&& );

        inline aqmd3::digitizer& digitizer() { return *digitizer_; }

    private:
        // bool handle_waveform( std::shared_ptr< const waveform >&& );

        std::unique_ptr< adportable::semaphore > sema_;
        boost::asio::io_service io_service_;
        std::vector< std::thread > threads_;
        std::queue< std::string > queue_;
        std::mutex mutex_;
        std::atomic<uint16_t> sequence_number_;
        std::vector< std::shared_ptr< adacquire::Receiver > > connections_;
        std::shared_ptr< adacquire::MasterObserver > masterObserver_;
        std::shared_ptr< aqmd3::WaveformObserver > waveformObserver_;
        std::shared_ptr< aqmd3::pkdObserver > pkdObserver_;
        uint64_t clocks_;
        uint64_t triggers_;
        uint32_t adc_core_clock_;
        const std::chrono::system_clock::time_point uptime_;
        uint32_t pos_;
        bool inject_trigger_latch_;
        uint32_t inject_trigger_latch_count_;
        std::string hvdg_json_;
        std::unique_ptr< aqmd3::digitizer > digitizer_;

        void reply_handler( const std::string& method, const std::string& reply );
        bool waveform_handler( const aqmd3controls::waveform * ch1, const aqmd3controls::waveform * ch2, aqmd3controls::method& );
    };

}
