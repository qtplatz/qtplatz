// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include "bnc565.hpp"
#include "log.hpp"
#include "serialport.hpp"
#include "dgprotocols.hpp" // handle json
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <array>
#include <atomic>
#include <fcntl.h>

#if !defined WIN32
#include <sys/mman.h>
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sstream>
#include <iostream>
#include <cstdint>

extern int __verbose_level__;
extern bool __debug_mode__;

namespace dg {

    std::atomic_flag lock = ATOMIC_FLAG_INIT;

    const uint32_t resolution = 10;

    inline uint32_t

    seconds_to_device( const double& t ) {
        return static_cast< uint32_t >( ( t * 1.0e9 ) / resolution + 0.5 );
    }

    inline double
    device_to_seconds( const uint32_t& t ) {
        return ( t * resolution ) * 1.0e-9;
    }
}

using namespace dg;

bnc565 *
bnc565::instance()
{
    static bnc565 __instance;
    return &__instance;
}

bnc565::operator bool () const
{
    return true;
}

bnc565::~bnc565()
{
    timer_.cancel();
    
    io_service_.stop();

    for ( auto& t: threads_ )
        t.join();
    
    log() << "memmap closed";
}

bnc565::bnc565() : fd_( 0 )
                 , timer_( io_service_ )
                 , tick_( 0 )
                 , deviceType_( NONE )
                 , deviceModelNumber_( 0 )
                 , deviceRevision_( -1 )
{
    boost::system::error_code ec;

    timer_.expires_from_now( std::chrono::milliseconds( 1000 ) );
    timer_.async_wait( [this]( const boost::system::error_code& ec ){ on_timer(ec); } );
    
    threads_.push_back( std::thread( [=]{ io_service_.run(); } ) );
}

void
bnc565::on_timer( const boost::system::error_code& ec )
{
    if ( !ec ) {
        ++tick_;
        handler_( tick_ );
    }
    timer_.expires_from_now( std::chrono::milliseconds( 1000 ) );
    timer_.async_wait( [this]( const boost::system::error_code& ec ){ on_timer(ec); } );    
}

boost::signals2::connection
bnc565::register_handler( const tick_handler_t::slot_type & subscriber )
{
    return handler_.connect( subscriber );
}

std::pair<double, double>
bnc565::pulse( uint32_t channel ) const
{
    // if ( channel < constants::pulse_addrs.size() && mapped_ptr_ ) {

    //     auto& lhs = constants::pulse_addrs[ channel ];

    //     uint32_t delay = mapped_ptr_[ lhs.first  ];
    //     uint32_t width = mapped_ptr_[ lhs.second ] - delay;

    //     if ( __verbose_level__ >= log::INFO ) {                
    //         log() << boost::format( "actual pulse[0x%08x] = %8d, [0x%08x] = %8d" )
    //             % ( lhs.first * sizeof(int32_t) ) % delay
    //             % ( lhs.second * sizeof(int32_t ) ) % ( delay + width );
    //     }

    //     return std::make_pair( device_to_seconds(delay), device_to_seconds( width ) );
    // }
    return std::make_pair(0, 0);
}

void
bnc565::setPulse( uint32_t channel, const std::pair<double, double>& t )
{
    // if ( dgmod_ == 0 ) {
    
    //     if ( channel < constants::pulse_addrs.size() && mapped_ptr_ ) {

    //         uint32_t delay = seconds_to_device( t.first );
    //         uint32_t width = seconds_to_device( t.second );

    //         auto& lhs = constants::pulse_addrs[ channel ];

    //         mapped_ptr_[ lhs.first  ] = delay;
    //         mapped_ptr_[ lhs.second ] = delay + width;

    //         if ( __verbose_level__ >= log::INFO ) {
    //             log() << boost::format( "pulse[0x%08x] := %8d, [0x%08x] := %8d" )
    //                 % ( lhs.first * sizeof(int32_t) ) % delay
    //                 % ( lhs.second * sizeof(int32_t ) ) % ( delay + width );
    //         }
    //     }
    // }
}

void
bnc565::commit( const dg::protocols<>& )
{
}

bool
bnc565::fetch( dg::protocols<>& ) const
{
    return true;
}

uint32_t
bnc565::revision_number() const
{
    return deviceRevision_;
}

const std::string&
bnc565::serialDevice() const
{
    return ttyname_;
}

// bool
// bnc565::peripheral_initialize()
// {
//     for ( int i = 0; i < 4; ++i ) {
//         std::string devname = ( boost::format( "/dev/ttyUSB%1%" ) % i ).str();
//         if ( boost::filesystem::exists( devname ) ) {
//             if ( initialize( devname ) && reset() )
//                 return true;
//         }
//     }
//     return false;
// }

bool
bnc565::peripheral_terminate()
{
    if ( usb_ ) {
        usb_->close();
        usb_.reset();
    }

    io_service_.stop();

    for ( auto& t: threads_ )
        t.join();

    threads_.clear();

    return true;
}

bool
bnc565::peripheral_query_device_data()
{
    std::lock_guard< std::mutex > lock( xlock_ );

    std::string reply;
    if ( idn_.empty() ) {
        if ( _xsend( "*IDN?\r\n", reply ) && reply[0] != '?' )
            idn_ = reply;
    }
    if ( inst_full_.empty() ) {
        if ( _xsend( "INST:FULL?\r\n", reply ) && reply[0] != '?' )
            inst_full_ = reply;
    }
    
    // To status (on|off)
    if ( _xsend( ":PULSE0:STATE?\r\n", reply ) ) {
        if ( reply[0] != '?' ) {
            try {
                int value = boost::lexical_cast<int>(reply);
                state0_ = value;
            } catch ( std::exception& ex ) {
                // Logger( L"%1% line#%2% exception %3% \"%4%\"", 
                //         EventLog::pri_INFO, L"0x100" ) % __FILE__ % __LINE__ % ex.what() % reply;
            }
        }
    }
    
    for ( int i = 0; i < int(states_.size()); ++i ) {
        const char * loc = "";
        try {
            if ( _xsend( ( boost::format( ":PULSE%1%:STATE?\r\n" ) % (i+1) ).str().c_str(), reply ) ) {
                loc = "STATE";
                int value = boost::lexical_cast<int>(reply);
                states_[ i ].state = value;
            }
            if ( _xsend( ( boost::format( ":PULSE%1%:WIDTH?\r\n" ) % (i+1) ).str().c_str(), reply ) ) {
                loc = "WIDTH";
                double value = boost::lexical_cast<double>(reply);
                states_[ i ].width = value;
            }
            if ( _xsend( ( boost::format( ":PULSE%1%:DELAY?\r\n" ) % (i+1) ).str().c_str(), reply ) ) {
                loc = "DELAY";
                double value = boost::lexical_cast<double>(reply);
                states_[ i ].delay = value;
            }
        } catch ( boost::bad_lexical_cast& ex ) {
            // Logger( L"%1% line#%2% exception %3% \"%4%\" for %5%"
            //         , EventLog::pri_INFO, L"0x100" ) % __FILE__ % __LINE__ % ex.what() % reply % loc;
        }
    }

    return true;
}

///
void
bnc565::handle_receive( const char * data, std::size_t length )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    while ( length-- ) {
        if ( std::isprint( *data ) ) {
            receiving_data_ += (*data);
        } else if ( *data == '\r' ) {
            ;
        } else if ( *data == '\n' ) {
            que_.push_back( receiving_data_ );
            receiving_data_.clear();
            cond_.notify_one();
        }
        ++data;
    }
}

///
bool
bnc565::xsend( const char * data, std::string& reply )
{
    bool res = _xsend( data, reply );

    std::string text( data );
    std::size_t pos = text.find_first_of( "\r" );
    if ( pos != std::string::npos )
        text.replace( text.begin() + pos, text.end(), "" );

    return res;
}

bool
bnc565::_xsend( const char * data, std::string& reply )
{
    reply.clear();

    if ( usb_ ) {

        std::unique_lock< std::mutex > lock( mutex_ );

        if ( usb_->write( data, std::strlen( data ), 20000 ) ) { // write with timeout(us)
            xsend_timeout_c_ = 0;

            if ( cond_.wait_for( lock, std::chrono::microseconds( 200000 ) ) != std::cv_status::timeout ) {
                reply_timeout_c_ = 0;
                if ( que_.empty() ) {
                    //adportable::debug(__FILE__, __LINE__) << "cond.wait return with empty que"; 
                    return false;
                }

                reply = que_.front();
                if ( que_.size() > 1 ) {
                    //adportable::debug(__FILE__, __LINE__) << "got " << que_.size() << " replies";
                    //for ( const auto& s: que_ )
                        //adportable::debug(__FILE__, __LINE__) << "\t" << s;
                }
                que_.clear();
                return true;
            } else {
                reply_timeout_c_++;
            }
        } else {
            xsend_timeout_c_++;
        }
    }
    return false;
}

bool
bnc565::_xsend( const char * data, std::string& reply, const std::string& expect, size_t ntry )
{
    while ( ntry-- ) {
        if ( _xsend( data, reply ) && reply == expect )
            return true;
    }
    return false;
}

bool
bnc565::initialize( const std::string& ttyname )
{
    xsend_timeout_c_ = 0;
    reply_timeout_c_ = 0;

    usb_ = std::make_unique< serialport >( io_service_ );

    if ( ! usb_->open( ttyname.c_str(), 115200 ) ) {
        return false;
    }

    ttyname_ = ttyname;

    usb_->async_reader( [=]( const char * send, std::size_t length ){ handle_receive( send, length ); } );
    usb_->start();

    threads_.push_back( std::thread( boost::bind( &boost::asio::io_service::run, &io_service_ ) ) );

    std::string reply;

    if ( _xsend( "*IDN?\r\n", reply ) && reply[0] != '?' ) { // identify
        idn_ = reply;
    }

    if ( _xsend( ":INST:FULL?\r\n", reply ) && reply[0] != '?' ) {
        inst_full_ = reply;
    }

    peripheral_query_device_data();

    return true;
}

bool
bnc565::reset()
{
    xsend_timeout_c_ = 0;
    reply_timeout_c_ = 0;

    // adportable::debug(__FILE__, __LINE__) << "BNC555::reset";

    std::string reply;

    if ( _xsend( "*RST\r\n", reply, "ok", 10 ) ) {

        for ( int ch  = 1; ch <= 5; ++ch ) {
            
            _xsend( (boost::format(":PULSE%1%:STATE ON\r\n") % ch).str().c_str(), reply, "ok", 10 );
            _xsend( (boost::format(":PULSE%1%:POL NORM\r\n") % ch).str().c_str(), reply, "ok", 10 );

        }
        
        if ( _xsend( ":PULSE5:SYNC CHA\r\n", reply, "ok", 10 ) &&
             _xsend( ":PULSE5:WIDTH 0.000001\r\n", reply, "ok", 10 ) && // 1us pulse for AP240 TRIG-IN
             _xsend( ":PULSE0:STATE ON\r\n", reply, "ok", 10 ) ) { // start trigger

             // simulated ion peak pulse
            _xsend( ":PULSE6:SYNC CHD\r\n", reply, "ok", 10 );
            _xsend( ":PULSE6:STATE ON\r\n", reply, "ok", 10 );
            _xsend( ":PULSE6:POL INV\r\n",  reply, "ok", 10 );
            _xsend( ":PULSE6:WIDTH 4.0E-9\r\n", reply, "ok", 10 ); // 4.0ns
            _xsend( ":PULSE6:DELAY 1.8E-6\r\n", reply, "ok", 10 ); // EXIT + 1.8us
            //_xsend( ":PULSE6:OUTP:MODE ADJ\r\n", reply, "ok", 10 ); // Adjustable
            //_xsend( ":PULSE6:OUTP:AMPL 2.3\r\n", reply, "ok", 10 ); // 2.3V
            //_xsend( ":SYST:KLOCK ON\r\n", reply, "ok", 10 ); // Locks the keypad
            _xsend( ":SYST:KLOCK OFF\r\n", reply, "ok", 10 );

            return true;
        }
    }
    return false;
}

void
bnc565::setInterval( double )
{
}
