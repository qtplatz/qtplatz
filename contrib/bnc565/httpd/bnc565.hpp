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

class serialport;

namespace dg {

    class bnc565 { // : public std::enable_shared_from_this< bnc565 > {
        bnc565();
    public:
        ~bnc565();

        enum DeviceType { NONE, HELIO, DE0 };

        static bnc565 * instance();

        operator bool () const;

        void setSerialDevice( const char * tty );
        const std::string& serialDevice() const;

        void setPulse( uint32_t channel, const std::pair< double, double >& );
        std::pair<double, double> pulse( uint32_t channel ) const;
        
        //double interval() const;
        
        //void commit();
        //void activate_trigger();
        //void deactivate_trigger();
        //uint32_t trigger() const;

        typedef boost::signals2::signal< void( size_t ) > tick_handler_t;

        boost::signals2::connection register_handler( const tick_handler_t::slot_type& );

        uint32_t revision_number() const;

        void commit( const dg::protocols<>& );
        bool fetch( dg::protocols<>& ) const;

        inline boost::asio::io_service& io_service() { return io_service_; }

        //bool peripheral_initialize();
        bool peripheral_terminate();
        bool peripheral_query_device_data();

        struct Channel {
            bool state;
            double width;
            double delay;
        };

        struct DelaySetpoint {
            bool state;
            double width; // s
            double delay; // s
            uint8_t channel;
            std::string name;
            DelaySetpoint() : state(false), width(0), delay(0), channel(0) {}
            DelaySetpoint( const char * _name ) : state(false), width(0), delay(0), channel(0), name(_name) {
            }
            DelaySetpoint( const DelaySetpoint& t ) : state(t.state), width(t.width)
                                                    , delay(t.width), channel(t.channel)
                                                    , name(t.name) {
            }
        };

        const std::vector<Channel>& state() const { return states_; };

        void setInterval( double );
        double interval() const { return interval0_; }
        bool state0() const { return state0_; }

        bool xsend( const char * data, std::string& );
        
    private:
        DeviceType deviceType_;
        int fd_;
        size_t tick_;
        tick_handler_t handler_; // tick handler
        boost::asio::io_service io_service_;
        boost::asio::steady_timer timer_; // interrupts simulator
        std::vector< std::thread > threads_;
        int deviceModelNumber_;
        int deviceRevision_;
        void on_timer( const boost::system::error_code& ec );
        //
        std::vector<Channel> states_;
        std::vector< DelaySetpoint > setpts_;
        double interval0_;
        bool state0_;
        std::unique_ptr< serialport > usb_;
        std::string idn_;
        std::string inst_full_;
        std::condition_variable cond_;
        std::mutex mutex_;
        std::mutex xlock_;
        std::string receiving_data_;
        std::vector< std::string > que_;
        std::string ttyname_;
        std::atomic< size_t > xsend_timeout_c_;
        std::atomic< size_t > reply_timeout_c_;

        bool _xsend( const char * data, std::string& );
        bool _xsend( const char * data, std::string&, const std::string& expect, size_t ntry );
        void handle_receive( const char * data, std::size_t length );
        bool initialize( const std::string& );
        bool reset();
    };
}

