/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include <thread>
#include <vector>

namespace TOF { struct ControlMethod; } // defined in tof.idl

namespace tofservant {

    class data_simulator;

    class avgr_emu {
    public:
        avgr_emu();
        bool peripheral_initialize();
        bool peripheral_terminate();
        bool peripheral_async_apply_method( const TOF::ControlMethod& m );
		bool peripheral_event_out( unsigned long );

        inline boost::asio::io_service& io_service() { return io_service_; }
    private:
        void initiate_timer();
        void handle_timeout( const boost::system::error_code& );

        void set_interval( size_t );
        void set_resolving_power( size_t );
        void set_num_average( size_t );

        size_t interval_;
		size_t trigger_event_out_;
        size_t npos_;
        size_t navg_;
        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
        boost::asio::deadline_timer timer_;
        std::unique_ptr< data_simulator > data_simulator_;
        std::vector< std::thread > threads_;
    };

}

