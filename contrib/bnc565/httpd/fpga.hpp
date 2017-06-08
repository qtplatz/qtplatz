// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "dgprotocols.hpp"
#include <atomic>
#include <memory>
#include <utility>
#include <cstdint>
#include <thread>
#include <vector>
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/signals2.hpp>

// namespace adportable { namespace dg { class protocols; } }

namespace dg {

    class fpga { // : public std::enable_shared_from_this< fpga > {
        fpga();
    public:
        ~fpga();

        enum DeviceType { NONE, HELIO, DE0 };

        static fpga * instance();

        operator bool () const;
        bool has_dgmod() const;

        void setPulse( uint32_t channel, const std::pair< double, double >& );
        std::pair<double, double> pulse( uint32_t channel ) const;
        
        void setInterval( double );
        double interval() const;
        
        void commit();
        void activate_trigger();
        void deactivate_trigger();
        uint32_t trigger() const;

        typedef boost::signals2::signal< void( size_t ) > tick_handler_t;

        boost::signals2::connection register_handler( const tick_handler_t::slot_type& );

        uint32_t revision_number() const;

        void commit( const adio::dg::protocols<>& );
        bool fetch( adio::dg::protocols<>& ) const;
        DeviceType deviceType() const;

        inline boost::asio::io_service& io_service() { return io_service_; }

    private:
        volatile uint32_t * mapped_ptr_;
        DeviceType deviceType_;
        int dgmod_;
        int fd_;
        size_t tick_;
        tick_handler_t handler_; // tick handler
        boost::asio::io_service io_service_;
        boost::asio::steady_timer timer_; // interrupts simulator
        std::vector< std::thread > threads_;
        int deviceModelNumber_;
        int deviceRevision_;
        void on_timer( const boost::system::error_code& ec );
    };
}

