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

#include "fpga.hpp"
#include "log.hpp"

//#include <dgmod/dgmod_delay_pulse.h> // kernel driver interface
//#include <adportable/debug.hpp>
#include "dgprotocols.hpp" // handle json
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
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

//static dgmod_protocol_sequence __sequence; // for httpd debugging on win32

#include <sstream>
#include <iostream>
#include <cstdint>

extern int __verbose_level__;
extern bool __debug_mode__;

namespace dg {

    static const uint32_t MAP_SIZE = 0x20000;
    static const uint32_t MAP_BASE_ADDR = 0xff200000;
    static const uint32_t PIO_BASE = 0x10040;
    std::atomic_flag lock = ATOMIC_FLAG_INIT;

    namespace constants {

        enum fsm_state { stop = 0x0000, start = 0x0001, update = 0x0001 };

        enum adddr {
            addr_machine_state = 0x10100 / sizeof(uint32_t)
            , addr_submit   = 0x10120 / sizeof(uint32_t)
            , addr_interval = 0x10180 / sizeof(uint32_t)
            , addr_revision = 0x100a0 / sizeof(uint32_t)
        };
        
        static std::array< const std::pair<uint32_t, uint32_t>, 6 > const pulse_addrs {
            std::make_pair(   0x10200u / sizeof(uint32_t)        // CH-0.delay      [0]
                            , 0x10220u / sizeof(uint32_t) )
          , std::make_pair(   0x10240u / sizeof(uint32_t)        // CH-1.delay      [1]
                            , 0x10260u / sizeof(uint32_t) )
          , std::make_pair(   0x10280u / sizeof(uint32_t)        // CH-2.delay      [2]
                            , 0x102a0u / sizeof(uint32_t) )
          , std::make_pair(   0x102c0u / sizeof(uint32_t)        // CH-3(0).delay   [3]
                            , 0x102e0u / sizeof(uint32_t) )
          , std::make_pair(   0x103c0u / sizeof(uint32_t)        // CH-3(1).delay   [4]
                            , 0x103e0u / sizeof(uint32_t) )
          , std::make_pair(   0x10400u / sizeof(uint32_t)        // CH-4.delay (ADC trigger delay) [5]
                            , 0x10420u / sizeof(uint32_t) )
                };
    }

    const uint32_t resolution = 10;

    inline uint32_t
    seconds_to_device( const double& t ) {
        return static_cast< uint32_t >( ( t * 1.0e9 ) / resolution + 0.5 );
    }

    inline double
    device_to_seconds( const uint32_t& t ) {
        return ( t * resolution ) * 1.0e-9;
    }

    // void print_xprotocol( const dgmod_protocol_sequence& sequence, const std::string& msg ) {
    //     std::cout << "---------------- xprotocol ---------------->" << std::endl;
    //     std::cout << msg << std::endl;
    //     for ( auto& xproto: sequence.protocols_ ) {
            
    //         std::cout << "rep: " << xproto.replicates_;
    //         for ( int ch = 0; ch < number_of_channels; ++ch )
    //             std::cout << boost::format( "{ %d, %d }" ) % xproto.delay_pulses_[ ch ].delay_ % xproto.delay_pulses_[ ch ].width_;
    //         std::cout << std::endl;
    //     }
    //     std::cout << "<---------------- xprotocol ----------------" << std::endl;
    // }

}

using namespace dg;

fpga *
fpga::instance()
{
    static fpga __instance;
    return &__instance;
}

fpga::operator bool () const
{
    return dgmod_ || mapped_ptr_;
}

bool
fpga::has_dgmod() const
{
    return dgmod_;
}

fpga::~fpga()
{
    timer_.cancel();
    
    io_service_.stop();

    for ( auto& t: threads_ )
        t.join();
    
#if defined __arm__
	if ( mapped_ptr_ )
        munmap( (void *)( mapped_ptr_ ), MAP_SIZE );
    if ( fd_ > 0 )
        close( fd_ );
#else 
    delete mapped_ptr_;
#endif
    log() << "memmap closed";
}

fpga::fpga() : mapped_ptr_( 0 )
             , fd_( 0 )
             , timer_( io_service_ )
             , tick_( 0 )
             , dgmod_( 0 )
             , deviceType_( NONE )
             , deviceModelNumber_( 0 )
             , deviceRevision_( -1 )
{
    boost::system::error_code ec;

#if defined __arm__
    if ( boost::filesystem::exists( "/dev/dgmod0", ec ) ) {
        deviceType_ = DE0;
        dgmod_ = open( "/dev/dgmod0", O_RDONLY );
        if ( dgmod_ < 0 ) {
            if ( dgmod_ == -EACCES )
                log() << "Insufficient permission to /dev/dgmod0 -- try 'sudo chmod +rw /dev/dgmod0'";
            else
                log() << boost::format( "/dev/dgmod0 open failed: %d" ) % (-dgmod_);
        } else {
            dgmod_dg_data model = { 0, 0 }, revision = { 1, 0 };
            if ( ioctl( dgmod_, DGMOD_GET_DG_DATA_0, &model ) == 0 &&
                 ioctl( dgmod_, DGMOD_GET_DG_DATA_0, &revision ) == 0 ) {
                deviceModelNumber_ = model.user_data_;
                deviceRevision_    = revision.user_data_;
            } else 
                log() << "ioctl to /dev/dgmod0 failed";            
            log() << boost::format( "/dev/dgmod0 open success. model# 0x%x\trevision %d" ) % deviceModelNumber_ % deviceRevision_;
        }
    } else {
        if ( ( fd_ = open( "/dev/mem", O_RDWR | O_SYNC ) ) > 0 ) {
            mapped_ptr_ = reinterpret_cast<uint32_t *> ( mmap( 0
                                                               , MAP_SIZE
                                                               , PROT_READ | PROT_WRITE
                                                               , MAP_SHARED
                                                               , fd_
                                                               , MAP_BASE_ADDR ) );
            deviceType_ = HELIO;
            deviceRevision_ = mapped_ptr_[ constants::addr_revision ];
            log() << boost::format( "/dev/mem open success. model# 0x%x\trevision %d" ) % deviceModelNumber_ % deviceRevision_;
        } else {
            log( log::ERR ) << boost::format( "failed to open /dev/mem: %d" ) % fd_;
        }
    }
#else
    do {
        // for simulator 
        mapped_ptr_ = new uint32_t [ 0x20000 / sizeof( uint32_t ) ];
        std::fill( mapped_ptr_, mapped_ptr_ + 0x20000 / sizeof( uint32_t ), 0 );
    } while ( 0 );
#endif

    timer_.expires_from_now( std::chrono::milliseconds( 1000 ) );
    timer_.async_wait( [this]( const boost::system::error_code& ec ){ on_timer(ec); } );
    
    threads_.push_back( std::thread( [=]{ io_service_.run(); } ) );
}

fpga::DeviceType
fpga::deviceType() const
{
    return deviceType_;
}

void
fpga::on_timer( const boost::system::error_code& ec )
{
    if ( !ec ) {
        ++tick_;
        handler_( tick_ );
    }
    timer_.expires_from_now( std::chrono::milliseconds( 1000 ) );
    timer_.async_wait( [this]( const boost::system::error_code& ec ){ on_timer(ec); } );    
}

boost::signals2::connection
fpga::register_handler( const tick_handler_t::slot_type & subscriber )
{
    return handler_.connect( subscriber );
}

void
fpga::setInterval( double t )
{
    uint32_t count = seconds_to_device( t );

    if ( mapped_ptr_ ) {

        mapped_ptr_[ constants::addr_interval ] = count;

        if ( __verbose_level__ >= log::INFO ) {
            log() << boost::format( "  T0 [0x%08x] := %8d" )
                % ( constants::addr_interval * sizeof(int32_t) ) % count;
        }

    }
}

double
fpga::interval() const
{
    uint32_t count(0);
    
    if ( mapped_ptr_ ) {

        count = mapped_ptr_[ constants::addr_interval ];

        if ( __verbose_level__ >= log::INFO ) {        
            log() << boost::format( "actual  T0 [0x%08x] = %8d" )
                % ( constants::addr_interval * sizeof(int32_t) ) % count;
        }
        
    }
    return device_to_seconds( count );
}

std::pair<double, double>
fpga::pulse( uint32_t channel ) const
{
    if ( channel < constants::pulse_addrs.size() && mapped_ptr_ ) {

        auto& lhs = constants::pulse_addrs[ channel ];

        uint32_t delay = mapped_ptr_[ lhs.first  ];
        uint32_t width = mapped_ptr_[ lhs.second ] - delay;

        if ( __verbose_level__ >= log::INFO ) {                
            log() << boost::format( "actual pulse[0x%08x] = %8d, [0x%08x] = %8d" )
                % ( lhs.first * sizeof(int32_t) ) % delay
                % ( lhs.second * sizeof(int32_t ) ) % ( delay + width );
        }

        return std::make_pair( device_to_seconds(delay), device_to_seconds( width ) );
    }
    return std::make_pair(0, 0);
}

void
fpga::setPulse( uint32_t channel, const std::pair<double, double>& t )
{
    if ( dgmod_ == 0 ) {
    
        if ( channel < constants::pulse_addrs.size() && mapped_ptr_ ) {

            uint32_t delay = seconds_to_device( t.first );
            uint32_t width = seconds_to_device( t.second );

            auto& lhs = constants::pulse_addrs[ channel ];

            mapped_ptr_[ lhs.first  ] = delay;
            mapped_ptr_[ lhs.second ] = delay + width;

            if ( __verbose_level__ >= log::INFO ) {
                log() << boost::format( "pulse[0x%08x] := %8d, [0x%08x] := %8d" )
                    % ( lhs.first * sizeof(int32_t) ) % delay
                    % ( lhs.second * sizeof(int32_t ) ) % ( delay + width );
            }
        }
    }
}

void
fpga::commit()
{
    if ( mapped_ptr_ ) {

        while ( lock.test_and_set( std::memory_order_acquire ) )
            ;

        mapped_ptr_[ constants::addr_submit ] = 0;
        mapped_ptr_[ constants::addr_submit ] = 0; // nop
        mapped_ptr_[ constants::addr_submit ] = 1;

        lock.clear( std::memory_order_release );

        uint32_t state = mapped_ptr_[ constants::addr_submit ];

        if ( __verbose_level__ >= log::INFO ) {
            log() << boost::format( " fsm [0x%08x] := 0x%02x" )
                % ( constants::addr_submit * sizeof(int32_t) ) % state;
        }
    }
}

uint32_t
fpga::revision_number() const
{
    return deviceRevision_;
}

void
fpga::activate_trigger()
{
    if ( mapped_ptr_ ) {
        mapped_ptr_[ constants::addr_machine_state ] = constants::start;
        
        log() << boost::format( " fsm [0x%08x] = 0x%02x" )
            % ( constants::addr_machine_state * sizeof(int32_t) ) % constants::start;
    }
}

void
fpga::deactivate_trigger()
{
    if ( mapped_ptr_ ) {
        mapped_ptr_[ constants::addr_machine_state ] = constants::stop;
        log() << boost::format( " fsm [0x%08x] = %02x" )
            % ( constants::addr_machine_state * sizeof(int32_t) ) % constants::stop;        
    }
}

uint32_t
fpga::trigger() const
{
	uint32_t value(0);

    if ( mapped_ptr_ ) {
        value = mapped_ptr_[ constants::addr_machine_state ];

        if ( __verbose_level__ >= log::INFO ) {        
            log() << boost::format( "actual fsm [0x%08x] = %02x" )
                % ( constants::addr_machine_state * sizeof(int32_t) ) % value;
        }
    }
    return value;
}

void
fpga::commit( const adio::dg::protocols<>& p )
{
    // dgmod_protocol_sequence sequence;
    // memset( &sequence, 0, sizeof( sequence ) );

    // sequence.size_ = std::min( sizeof( sequence.protocols_ )/sizeof( sequence.protocols_[ 0 ] ), p.size() );
    // sequence.interval_ = dg::seconds_to_device( p.interval() );
    // uint32_t protocolIndex(0);
    // for( const auto& proto: p ) {
    //     auto& xproto = sequence.protocols_[ protocolIndex++ ];
    //     if ( proto.replicates() == 0 ) {
    //         sequence.size_ = protocolIndex - 1;
    //         break;
    //     }
    //     xproto.replicates_ = proto.replicates();
    //     std::cout << "repl. " << proto.replicates();
    //     for ( size_t ch = 0; ch < number_of_channels; ++ch ) {
    //         std::cout << "ch-" << ch << " delay=" << proto.pulses()[ch].first << ", width=" << proto.pulses()[ch].second << std::endl;
    //         xproto.delay_pulses_[ ch ].delay_ = dg::seconds_to_device( proto.pulses()[ ch ].first );
    //         xproto.delay_pulses_[ ch ].width_ = dg::seconds_to_device( proto.pulses()[ ch ].second );
    //     }
    //     if ( protocolIndex >= sequence.size_ )
    //         break;
    // }

    // if ( dgmod_ > 0 ) {
    //     int errc = ioctl( dgmod_, DGMOD_SET_PROTOCOLS, &sequence );
    //     if ( !errc )
    //         log() << boost::format( "DGMOD_SET_PROTOCOLS failed with %d" ) % errc;
    // }

    // __sequence = sequence;

    // if ( __debug_mode__ )
    //     print_xprotocol( __sequence, "fpga::commit( 2 ) " );
}

#if defined __GNUC__
# pragma GCC diagnostic ignored "-Wenum-compare"
#endif

bool
fpga::fetch( adio::dg::protocols<>& lhs ) const
{
    // dgmod_protocol_sequence sequence;
    // memset( &sequence, 0, sizeof( sequence ) );
    
    // if ( ! dgmod_ && __debug_mode__ )
    //     sequence = __sequence;

    // if ( dgmod_ && ( ioctl( dgmod_, DGMOD_GET_PROTOCOLS, &sequence ) != 0 ) )
    //     log( log::ERR ) << "httpd::fpga failed to fetch from /dev/dgmod0.";
    
    // if ( __debug_mode__ )
    //     print_xprotocol( sequence, "fpga::fetch" );
    
    // lhs.setInterval( device_to_seconds( sequence.interval_ ) );
    // lhs.resize( sequence.size_ );

    // static_assert( number_of_channels == adio::dg::protocol<>::size, "" );

    // for ( size_t protocolIndex = 0; protocolIndex < lhs.size(); ++protocolIndex ) {

    //     const auto& xproto = sequence.protocols_[ protocolIndex ];
    //     auto it = lhs.begin() + protocolIndex;
        
    //     it->setReplicates( xproto.replicates_ );

    //     for ( int ch = 0; ch < number_of_channels; ++ch ) 
    //         (*it)[ch] = { device_to_seconds( xproto.delay_pulses_[ ch ].delay_ ), device_to_seconds( xproto.delay_pulses_[ ch ].width_ ) };

    // }

    return true;
}

