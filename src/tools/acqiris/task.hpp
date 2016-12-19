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
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/signals2.hpp>
#include <chrono>
#include <cstdint>
#include <future>
#include <memory>
#include <vector>

namespace acqrscontrols {
    namespace aqdrv4 {
        class acqiris_method;
        class acqiris_protocol;
        class waveform;
        enum SubMethodType : unsigned int;
    }
}

class digitizer;

class task {
    ~task();
    task();
    task( const task& ) = delete;
    const task& operator = ( const task& ) = delete;
public:

    static task * instance();
    static class digitizer * digitizer();
    
    bool initialize();
    bool finalize();

    inline boost::asio::io_service& io_service() { return io_service_; }    
    inline boost::asio::io_service::strand& strand() { return strand_; }

    void prepare_for_run( std::shared_ptr< const acqrscontrols::aqdrv4::acqiris_method >
                          , acqrscontrols::aqdrv4::SubMethodType );
    void event_out( uint32_t );

    bool digitizer_initialize();

    typedef boost::signals2::signal< void( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > ) > acqiris_method_adapted_t;
    typedef boost::signals2::signal< void( int ) > replyTemperature_t;

    boost::signals2::connection connect_acqiris_method_adapted( const acqiris_method_adapted_t::slot_type & subscriber );
    boost::signals2::connection connect_replyTemperature( const replyTemperature_t::slot_type & subscriber );
    void connect_push( std::function< void( std::shared_ptr< acqrscontrols::aqdrv4::waveform > ) > );

private:
    boost::asio::io_service io_service_;
    boost::asio::io_service::work work_;
    boost::asio::io_service::strand strand_;
    boost::asio::steady_timer timer_;
    std::atomic< bool > worker_stopping_;
    adportable::semaphore sema_;
    std::vector< std::thread > threads_;
    std::atomic_flag acquire_posted_;
    const std::chrono::time_point<std::chrono::system_clock> tp_uptime_;
    std::chrono::time_point<std::chrono::system_clock> tp_data_handled_;
    uint32_t methodNumber_;
    uint64_t inject_timepoint_;
    uint64_t inject_serialnumber_;
    bool inject_requested_;
    bool acquisition_active_;

    acqiris_method_adapted_t emit_acqiris_method_adapted_;
    replyTemperature_t emit_replyTemperature_;
    std::function< void( std::shared_ptr< acqrscontrols::aqdrv4::waveform > ) > push_handler_;

    void acquire();
    void worker_thread();
    void handle_timer( const boost::system::error_code& ec );
};
