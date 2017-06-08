// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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

#include "arpproxy.hpp"
#include "array_wrapper.hpp" // copied from adportable
#include "usbmanager.hpp"
#include "log.hpp"
#include <infitofdefns/arpvoltage.hpp>
#include <infitofdefns/avgr_arp.hpp>
#include <tofdll2/ddr2_trig.hpp>
#include <array>
#include <atomic>
#include <fcntl.h>

#ifdef WIN32
enum { LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED,LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT };
const char * libusb_error_name( int ) { return ""; }
int libusb_bulk_transfer( libusb_device_handle *, unsigned char ep, unsigned char * data, int length, int *transferred, unsigned int timeout ) { return 0; }
#endif

#ifndef WIN32
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#if defined __linux__ && HAS_LIBUSB
#  include <libusb-1.0/libusb.h>
#endif

#include <boost/format.hpp>
#include <atomic>
#include <cstdint>
#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>

extern int __verbose_level__;

namespace dg {

    enum endpoint {
        CmdBulkOut    = 0x01
        , DataBulkOut = 0x02
        , RegBulkOut  = 0x04
        , CmdBulkIn   = 0x81
        , DataBulkIn  = 0x86
        , RegBulkIn   = 0x88
    };

    enum COMMAND_VECTOR {
        ARP_TBLVAL_WRITE		
        , ARP_TBLVAL_READ		
        , ARP_TBLVAL_READWRITE	
        , ARP_TBLVAL_WAIT		
        , ARP_TBLVAL_COMP		
        , ARP_TBLVAL_COMP_N		
        , ARP_TBLVAL_COMP_L		
        , ARP_TBLVAL_COMP_H
    };

    enum {
        CMD_SFR_READ		= 0x10
        , CMD_SFR_WRITE		= 0x20
        , CMD_REG_READ		= 0x30
        , CMD_REG_WRITE		= 0x31
        , CMD_EP2468_INIT	= 0x34
        , CMD_EPx_FIFO_RESET = 0x35
        , CMD_EEPROM_READ	= 0x38
        , CMD_EEPROM_WRITE	= 0x39
        , CMD_MODIFIED_DATE	= 0x40
        , CMD_TCK_TOGGLE	= 0x44
        , CMD_IFCONFIG		= 0x40
        , ARP_DDR2_ADDR_TRIG = 0x38000000 // size := 0x0004B200
        , ARP_OFFSET_HV     = 0x00004800
    };

    struct Arp_TblValueDetail {
        int32_t iType;
        uint32_t iAddr;
        uint64_t llValue;
        uint32_t iMask;
        uint32_t iMaxLoop;
    };
    
    class arpproxy::impl {
    public:
        impl() : worker_( io_service_ )
               , timer_( io_service_ )
               , usbmanager_( new usbmanager() )
               , tick_( 1000 )
               , service_count_( 0 )
               , usb_device_handle_( 0 ) {

            log() << "initializing arp proxy...";
            
            std::fill( actuals_data_.begin(), actuals_data_.end(), -1 );
            std::fill( setpts_data_.begin(), setpts_data_.end(), std::make_pair( 0.0, 0 ) );

            usbmanager_->initialize( [this]( libusb_device * dev, libusb_hotplug_event event ){
                    if ( event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED ) {
                        io_service_.post( [this, dev]{ on_device_attached( dev ); } );
                    } else {
                        io_service_.post( [this, dev]{ on_device_detached( dev ); } );
                    }
                });

            log() << "starting timer...";            

            timer_.expires_from_now( std::chrono::milliseconds( 1000 ) );
            timer_.async_wait( [this]( const boost::system::error_code& ec ){ on_timer(ec); } );

            threads_.push_back( std::thread( [=]{ io_service_.run(); } ) );

            log() << "timer started.";            
        }

        ~impl() {
            timer_.cancel();
            io_service_.stop();
            for ( auto& t: threads_ )
                t.join();
        }

    public:
        std::function<void( size_t )> actuals_handler_;
        std::function<void( const std::string& )> notification_handler_;

        const std::pair< double, uint32_t >& setpoint( uint32_t addr ) const { return setpts_data_[ addr ]; }

        uint32_t actual( uint32_t addr ) const { return actuals_data_[ addr ]; }

        void device_setvoltage( const std::string& id, double value );
        void device_setflag( const std::string& id, bool value );        

    private:
        boost::asio::io_service io_service_;
        boost::asio::io_service::work worker_;
        boost::asio::steady_timer timer_;
        std::unique_ptr< usbmanager > usbmanager_;
        std::vector< std::thread > threads_;
        size_t tick_;
        libusb_device_handle * usb_device_handle_;
        std::atomic<int> service_count_;
        std::mutex mutex_;

        std::array< std::pair<double, uint32_t>, 32 > setpts_data_; // <user value, device value>
        std::array< uint32_t, 32 > actuals_data_;
        std::string ezusb_version_;
        
        void on_device_attached( libusb_device * dev ) {

            std::lock_guard< std::mutex > lock( mutex_ );

            if ( usbmanager::usb_open( dev, usb_device_handle_ ) ) {
                if ( usbmanager::usb_claim_interface( usb_device_handle_, 0 ) ) {
                    io_service_.post( [this]{ handle_device_initialize(); } );                    
                }
            }
        }
        
        void on_device_detached( libusb_device * dev ) {

            do {
                std::lock_guard< std::mutex > lock( mutex_ );
                usbmanager::usb_close( usb_device_handle_ );
            } while(0);
            
            notification_handler_( "{ \"notify\": [{ \"id\": \"status\", \"value\": \"offline\" } ]}" );
        }
        
        void on_timer( const boost::system::error_code& ec ) {
            if ( !ec ) {
                namespace arp = infitof::arp;
                
                std::lock_guard< std::mutex > lock( mutex_ );

                if ( usb_device_handle_ ) {
                
                    if ( tick_++ == 0 ) {
                        read_device_version();
                        io_service_.post( [this]{ handle_device_on( false ); } ); // HV Off
                        io_service_.post( [this]{ handle_device_digitizer_initialize(); } );
                    }

                    if ( service_count_ == 0 ) {
                        ++service_count_;
                        io_service_.post( [this]{ handle_device_read_actuals(); --service_count_; } );
                    }
                }

                timer_.expires_from_now( std::chrono::milliseconds( 1000 ) );
                timer_.async_wait( [this]( const boost::system::error_code& ec ){ on_timer(ec); } );
            }
        }

        void handle_device_initialize();
        void handle_device_digitizer_initialize();
        void handle_device_read_actuals();
        void handle_device_on( bool );
        void handle_device_setvoltage( uint32_t addr, double wold_value );
        void handle_device_setflag( uint32_t addr, uint32_t mask, bool value );
        void handle_device_setvoltages();
        void handle_device_fpga_reset();
        bool read_device_version();

        int bulk_transfer( endpoint ep, uint8_t * data, int length, int& transferred, uint32_t timeout = 1000 );
        int bulk_transfer( endpoint ep, const uint8_t * data, int length, int& transferred, uint32_t timeout = 1000 );

        void make_actual_status( std::ostream& );

        template<typename T> int bulk_transfer( endpoint ep, const T& data ) {
            int transferred(0);
            return bulk_transfer( ep, reinterpret_cast< const uint8_t * >(&data), sizeof(data), transferred, 1000 );
        }
        
        template<typename T> int bulk_transfer( endpoint ep, T& data, int& transferred ) {
            return bulk_transfer( ep, reinterpret_cast< uint8_t * >(&data), sizeof(data), transferred, 1000 );
        }

        template<typename InputIt> bool CmdRegVectorWrite( InputIt first, InputIt last ) {
            size_t count = std::distance( first, last );
            std::vector< uint32_t > outData( count * 3 );
            uint32_t * p = outData.data();
            std::for_each( first, last, [&] ( const typename InputIt::value_type& v ) {
                    *p++ = 0x0f000c15; *p++ = uint32_t( v.first );  *p++ = uint32_t( v.second ); } );
            return CmdRegVectorWriteHelper( outData.data(), count );
        }

        bool CmdRegRead( uint32_t addr, uint32_t &data );
        bool CmdRegWrite( uint32_t addr, uint32_t data );
        bool CmdRegBitWrite( uint32_t addr, uint32_t data, uint32_t mask );
        bool CmdRegVectorWrite( const std::vector< std::pair< int32_t, int32_t > >& data );
        bool VectorInterpreter( Arp_TblValueDetail *, size_t nitem );
        bool CmdRegVectorWriteHelper( const uint32_t *, size_t nitem );
    };

    namespace arp = infitof::arp;
    typedef arpproxy::impl impl;

    typedef std::pair<double, uint32_t > device_value_type;
    
    struct idHVSetpt {
        uint32_t addr;
        uint32_t mask;
        bool is_voltage;
        std::function< std::string( const device_value_type& device_value ) > to_string;
        std::function< uint32_t( double ) > device_value;
    };
    
    static std::map< const std::string, idHVSetpt > __idHVSetpts = {
        { "switch-hv"
          , { arp::setpt_aux1, uint32_t(-1), false
              , [] ( const device_value_type& d ) { return (d.second & 0x1fff) == 0x1fff ? "on" : "off"; }
              , [] ( double ) { return 0; /* n/a */ }
            }
        }
        , { "switch-isovalve"
            , { arp::setpt_pumpValveCtrl, 0x1000, false
                , [] ( const device_value_type& d ) { return d.second & 0x1000 ? "on" : "off"; }
                , [] ( double ) { return 0; /* n/a */ }
            }
        }
        , { "switch-stdinlet"
            , { arp::setpt_pumpValveCtrl, 0x0800, false
                , [] ( const device_value_type& d ) { return d.second & 0x0800 ? "on" : "off"; }
                , [] ( double ) { return 0; /* n/a */ }
            }
        }
        , { "switch-sampleinlet"
            , { arp::setpt_pumpValveCtrl, 0x0400, false
                , [] ( const device_value_type& d ) { return d.second & 0x0400 ? "on" : "off"; }
                , [] ( double ) { return 0; /* n/a */ }
            }
        }
        , { "switch-filsel"
            , { arp::setpt_aux2, 0x0001, false
                , [] ( const device_value_type& d ) { return d.second & 0x0001 ? "on" : "off"; }
                , [] ( double ) { return 0; /* n/a */ }
            }
        }
        , { "Vdetector.SET" // arp::setpt_detVoltage
            , { arp::setpt_detVoltage, 0, true
                , [] ( const device_value_type& d ) { return ( boost::format( "%.2lf" ) % d.first ).str(); }
                , [] ( double d ) { return arp::Vdet::device_value( d ); }
            }
        }
        //--
        , { "EJECTION.INNER.SET" // arp::setpt_extInVoltage
            , { arp::setpt_extInVoltage, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Vext_in::device_value( d ); }                
            }
        }
        , { "EJECTION.OUTER.SET" // arp::setpt_extOutVoltage
            , { arp::setpt_extOutVoltage, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Vext_out::device_value( d ); }                                
            }
        }
        //--
        , { "INJECTION.INNER.SET" // arp::setpt_entInVoltage
            , { arp::setpt_entInVoltage, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Vent_in::device_value( d ); }                                                
            }
        }
        , { "INJECTION.OUTER.SET" // arp::setpt_entOutVoltage
            , { arp::setpt_entOutVoltage, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Vent_out::device_value( d ); }                                                                
            }
        }
        //--
        , { "GATE.OUTER.SET" // arp::setpt_selOutVoltage
            , { arp::setpt_selOutVoltage, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Vsel_out::device_value( d ); }
            }
        }
        , { "GATE.INNER.SET" // arp::setpt_selOutVoltage
            , { arp::setpt_selInVoltage, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Vsel_in::device_value( d ); }
            }
        }
        //--
        , { "ORBIT.INNER.SET" // arp::setpt_turnInVoltage
            , { arp::setpt_turnInVoltage, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Vtn_in::device_value( d ); }                
            }
        }
        , { "ORBIT.OUTER.SET" // arp::setpt_turnOutVoltage
            , { arp::setpt_turnOutVoltage, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Vtn_out::device_value( d ); }
            }
        }
        , { "Vmatsuda.SET" // arp::setpt_turnMVoltage
            , { arp::setpt_turnMVoltage, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Vtn_m::device_value( d ); }
            }
        }
        , { "Veinzel.SET" // arp::setpt_stlVoltage
            , { arp::setpt_einzelVoltage, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Veinzel::device_value( d ); }                
            }
        }
        , { "Vacc.SET" // arp::setpt_accVoltage
            , { arp::setpt_accVoltage, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Vacc::device_value( d ); }
            }
        }
        , { "Vpush.SET" // arp::setpt_pushVoltage
            , { arp::setpt_pushVoltage, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Vpush::device_value( d ); }                
            }
        }
        , { "Afilament.SET" // arp::setpt_filCurrent
            , { arp::setpt_filCurrent, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Cfilament::device_value( d ); }
            }
        }
        , { "Vion.SET" // arp::setpt_ionVoltage
            , { arp::setpt_ionVoltage, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Vionization::device_value( d ); }                
            }
        }
        , { "H20W.SET" // arp::setpt_ionVoltage
            , { arp::setpt_heaterTemp20W, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Theat20::device_value( d ); }                
            }
        }
        , { "H50W.SET" // arp::setpt_ionVoltage
            , { arp::setpt_heaterTemp50W, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Theat50::device_value( d ); }                
            }
        }
        , { "H100W.SET" // arp::setpt_ionVoltage
            , { arp::setpt_heaterTemp100W, 0, true
                , [] ( const device_value_type& d ){ return ( boost::format("%.2lf") % d.first ).str(); }
                , [] ( double d ) { return arp::Theat100::device_value( d ); }                
            }
        }
        , { "switch-h20w" // arp::setpt_ionVoltage
            , { arp::setpt_aux2, 0x0020, false
                , [] ( const device_value_type& d ) { return ( d.second & 0x0020 ) ? "on" : "off"; }
                , [] ( double ) { return 0; /* n/a */ }
            }
        }
        , { "switch-h50w" // arp::setpt_ionVoltage
            , { arp::setpt_aux1, 0x2000, true
                , [] ( const device_value_type& d ) { return ( d.second & 0x2000 ) ? "on" : "off"; }
                , [] ( double ) { return 0; /* n/a */ }                
            }
        }
        , { "switch-h100w" // arp::setpt_ionVoltage
            , { arp::setpt_aux1, 0x4000, true
                , [] ( const device_value_type& d ){ return ( d.second & 0x4000 ) ? "on" : "off"; }
                , [] ( double ) { return 0; /* n/a */ }                                
            }
        }
    };

    struct setpt_handler {

        typedef std::pair< const std::string, idHVSetpt > value_type;
        typedef std::map< const std::string, idHVSetpt >::iterator iterator;
        typedef std::map< const std::string, idHVSetpt >::const_iterator const_iterator;        

        static std::map< const std::string, idHVSetpt >::const_iterator begin() { return __idHVSetpts.begin(); }
        static std::map< const std::string, idHVSetpt >::const_iterator end() { return __idHVSetpts.end(); }

        static std::map< const std::string, idHVSetpt >::const_iterator find( uint32_t addr ) {
            return std::find_if( begin(), end(), [addr] ( const value_type& x ) {
                    return x.second.addr == addr;
                } );
        }

        static std::map< const std::string, idHVSetpt >::iterator find( const std::string& id ) {
            return __idHVSetpts.find( id );
        }

        static bool empty( std::map< const std::string, idHVSetpt >::const_iterator it ) { return it == __idHVSetpts.end(); }

        static const std::string& id( std::map< const std::string, idHVSetpt >::const_iterator it ) { return it->first; }

    };

    static std::pair< const char *, std::function< std::string( const impl& x ) > > __actuals [] = {
		{ "INJECTION.OUTER.ACT", [=](const impl& x){ return ( boost::format( "%.2lf" ) % arp::Vent_out::world_value( x.actual( arp::act_entOutVoltage ) ) ).str(); } }
        , { "INJECTION.INNER.ACT", [=](const impl& x){ return ( boost::format("%.2lf") % arp::Vent_in::world_value( x.actual( arp::act_entInVoltage ) ) ).str(); } }
        , { "EJECTION.OUTER.ACT",  [=](const impl& x){ return ( boost::format("%.2lf") % arp::Vext_out::world_value( x.actual( arp::act_extOutVoltage ) ) ).str(); } }
        , { "EJECTION.INNER.ACT",  [=](const impl& x){ return ( boost::format("%.2lf") % arp::Vext_in::world_value( x.actual( arp::act_extInVoltage ) ) ).str(); } }
        , { "GATE.OUTER.ACT",      [=](const impl& x){ return ( boost::format("%.2lf") % arp::Vsel_out::world_value( x.actual( arp::act_selOutVoltage ) ) ).str(); } }
        , { "GATE.INNER.ACT",      [=](const impl& x){ return ( boost::format("%.2lf") % arp::Vsel_in::world_value( x.actual( arp::act_selInVoltage ) ) ).str(); } }
        , { "ORBIT.OUTER.ACT",     [=](const impl& x){ return ( boost::format("%.2lf") % arp::Vtn_out::world_value( x.actual( arp::act_turnOutVoltage ) ) ).str(); } }
        , { "ORBIT.INNER.ACT",     [=](const impl& x){ return ( boost::format("%.2lf") % arp::Vtn_in::world_value( x.actual( arp::act_turnInVoltage ) ) ).str(); } }
        , { "Vmatsuda.ACT",        [=](const impl& x){ return ( boost::format("%.2lf") % arp::Vtn_m::world_value( x.actual( arp::act_turnM ) ) ).str(); } }
        , { "Veinzel.ACT",         [=](const impl& x){ return ( boost::format("%.2lf") % arp::Veinzel::world_value( x.actual( arp::act_einzelVoltage ) ) ).str(); } }
        , { "Vacc.ACT",            [=](const impl& x){ return ( boost::format("%.2lf") % arp::Vacc::world_value( x.actual( arp::act_accVoltage ) ) ).str(); } }
        , { "Vpush.ACT",           [=](const impl& x){ return ( boost::format("%.2lf") % arp::Vpush::world_value( x.actual( arp::act_pushVoltage ) ) ).str(); } }
        , { "Afilament.ACT",       [=](const impl& x){ return ( boost::format("%.2lf") % arp::Cfilament::world_value( x.actual( arp::act_filCurrent ) ) ).str(); } }
        , { "Vion.ACT",            [=](const impl& x){ return ( boost::format("%.2lf") % arp::Vionization::world_value( x.actual( arp::act_ionVoltage ) ) ).str(); } }
        , { "Vdetector.ACT",       [=](const impl& x){ return ( boost::format("%.2lf") % arp::Vdet::world_value( x.actual( arp::act_detVoltage ) ) ).str(); } }
        , { "PressureGauge", [=] ( const impl& x ) {
                double u = arp::VpressGauge::world_value( x.actual( arp::act_guageMonitor ) );
                double p = std::pow( 10.0, ( ( u - 7.75 ) / 0.75 ) + 2 );
                return ( boost::format( "%.4e" ) % p ).str();
            }
        }
        , { "H20W.ACT",            [=]( const impl& x ){ return ( boost::format("%.2lf") % arp::Theat20::world_value( x.actual( arp::act_heater20WTemp ) ) ).str(); } }
        , { "H50W.ACT",            [=]( const impl& x ){ return ( boost::format("%.2lf") % arp::Theat50::world_value( x.actual( arp::act_heater50WTemp ) ) ).str(); } }
        , { "H100W.ACT",           [=]( const impl& x ){ return ( boost::format("%.2lf") % arp::Theat100::world_value( x.actual( arp::act_heater100WTemp ) ) ).str(); } }
    };
    
}

using namespace dg;

arpproxy::~arpproxy()
{
    delete impl_;
}

arpproxy::arpproxy() : impl_( new impl() )
{
}

arpproxy::operator bool () const
{
    return true;  // TODO -- reflect USB 
}

void
arpproxy::register_actuals_handler( std::function<void( size_t )> f )
{
    impl_->actuals_handler_ = f;
}

void
arpproxy::register_notification_handler( std::function<void( const std::string& )> f )
{
    impl_->notification_handler_ = f;
}

void
arpproxy::setpts_json_response( std::ostream& o )
{
    namespace arp = infitof::arp;

    o << "\"setpts\": [";
    size_t n = 0;
    std::for_each( __idHVSetpts.begin(), __idHVSetpts.end(), [&](const setpt_handler::value_type& x ) {
            if ( x.second.is_voltage ) {
                if ( n++ != 0 )
                    o << ", ";
                o << "{ \"id\": \"" << x.first << "\", \"value\": \"" << x.second.to_string( impl_->setpoint( x.second.addr ) ) << "\" }";
            }
        });
    o << "]";    
}

void
arpproxy::actuals_json_response( size_t tick, std::ostream& o )
{
    namespace arp = infitof::arp;


    o << "\"acts\": [";
    o << boost::format( "{ \"id\": \"hvtick\", \"value\": \"%d\" }" ) % tick;

	int n = 0;
    for ( auto a: __actuals ) {
		o << ", { \"id\": \"" << a.first << "\", \"value\": \"" << a.second(*impl_) << "\" }";
    }
    o << "]";
}

void
arpproxy::flags_json_response( std::ostream& json )
{
    json << "\"checkbox\": [ ";
    int n = 0;
    std::for_each( setpt_handler::begin(), setpt_handler::end(), [&]( const setpt_handler::value_type& x ){
            if ( ! x.second.is_voltage ) {
                if ( n++ )
                    json << ", ";
                json << boost::format( "{ \"id\": \"%s\", \"value\": \"%s\" }" ) % x.first % x.second.to_string( impl_->setpoint( x.second.addr ) );
            }
        });
    json << "]";
}

void
arpproxy::set( const std::string& item, double value )
{
    impl_->device_setvoltage( item, value );
}

void
arpproxy::set( const std::string& item, bool value )
{
    impl_->device_setflag( item, value );
}

////////////
bool
arpproxy::impl::VectorInterpreter( Arp_TblValueDetail * details, size_t nitem )
{
	bool	bResult( true );
    
    adportable::array_wrapper< Arp_TblValueDetail > vec( details, nitem );

    auto it = vec.begin();
    
    while ( it != vec.end() ) {
        if ( it->iType == ARP_TBLVAL_WRITE ) {
            std::vector< std::pair< uint32_t, uint32_t > > xdata;
            xdata.push_back( std::make_pair( it->iAddr, uint32_t( it->llValue ) ) );
            while ( ++it != vec.end() && it->iType == ARP_TBLVAL_WRITE )
                xdata.push_back( std::make_pair( it->iAddr, uint32_t( it->llValue ) ) );
            --it;
            if ( !CmdRegVectorWrite( xdata.begin(), xdata.end() ) )
                return false;
        } else if ( it->iType == ARP_TBLVAL_READ ) {
            uint32_t value;
            if ( CmdRegRead( uint32_t(it->iAddr), value ) )
                it->llValue = value;
            else
                return false;
        } else if ( it->iType == ARP_TBLVAL_READWRITE ) {
            int shift_count = 0;
            for ( int j = 0 ; ( (it->iMask >> j) & 0x01 ) == 0 ; j++ )
                shift_count = j + 1;
            if ( ! CmdRegBitWrite( it->iAddr, uint32_t(it->llValue << shift_count), uint32_t(it->iMask)) )
                return false;
        } else if ( it->iType == ARP_TBLVAL_WAIT ) {
            std::this_thread::sleep_for( std::chrono::microseconds( it->llValue ) );
        } else if ( it->iType == ARP_TBLVAL_COMP ) {
            uint32_t l_iComp;
            uint32_t l_iLoop = 0;
            do {
                if (it->iMaxLoop < l_iLoop ) {
                    bResult	= false;
                    break;
                }
                std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
                bResult = CmdRegRead( it->iAddr, l_iComp);
                l_iLoop++;
            } while ( (bResult != false) && ((l_iComp & it->iMask) != uint32_t( it->llValue )) );

        } else if ( it->iType == ARP_TBLVAL_COMP_N ) {
            uint32_t l_iComp;
            uint32_t l_iLoop = 0;
            do {
                if ( it->iMaxLoop < l_iLoop )  {
                    bResult = false;
                    break;
                }
                std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
                bResult = CmdRegRead( it->iAddr, l_iComp);
                l_iLoop++;
            } while ( (bResult != false) && ((l_iComp & it->iMask) == int( it->llValue )) );
        }
        ++it;
    }
	return bResult;
}

bool
arpproxy::impl::CmdRegRead( uint32_t addr, uint32_t &data )
{
    uint8_t oframe[] = {
        0x14
        , 0x08
        , 0
        , 0
        , uint8_t( (unsigned(addr) >>  0) & 0xff )
        , uint8_t( (unsigned(addr) >>  8) & 0xff )
        , uint8_t( (unsigned(addr) >> 16) & 0xff )
        , uint8_t( (unsigned(addr) >> 24) & 0xff )
    };

    int transferred;
    int rcode = bulk_transfer( RegBulkOut, oframe );
    if ( rcode == 0 ) {
        uint8_t rdata[8];
        
        rcode = bulk_transfer( RegBulkIn, rdata, sizeof( rdata ), transferred );
        if ( rcode == 0 ) {
            data = ( rdata[7] << 24 ) | ( rdata[6] << 16 ) | ( rdata[5] << 8 ) | rdata[ 4 ];
            return true;
        }
        // it may need clear_halt
    }
    log() << boost::format( "CmdRegRead( 0x%x:0x%x ) %s." ) % addr % data % libusb_error_name( rcode );    
    return false;
}

bool
arpproxy::impl::CmdRegWrite( uint32_t addr, uint32_t data )
{
    uint8_t oframe[] = {
        0x15, 0x0C, 0, 0x0F
        , uint8_t( (unsigned(addr) >>  0) & 0xff)
        , uint8_t( (unsigned(addr) >>  8) & 0xff)
        , uint8_t( (unsigned(addr) >> 16) & 0xff)
        , uint8_t( (unsigned(addr) >> 24) & 0xff)
        , uint8_t( (unsigned(data) >>  0) & 0xff)
        , uint8_t( (unsigned(data) >>  8) & 0xff)
        , uint8_t( (unsigned(data) >> 16) & 0xff)
        , uint8_t( (unsigned(data) >> 24) & 0xff)
    };

    int transferred;
    int rcode = bulk_transfer( RegBulkOut, oframe, sizeof( oframe ), transferred );
    if ( rcode == 0 ) {
        uint8_t iframe[8];
        rcode = bulk_transfer( RegBulkIn, iframe, sizeof( iframe ), transferred );
        if ( rcode == 0 )
            return true;
    }

    log() << boost::format( "CmdRegWrie( 0x%x:0x%x ) %s." ) % addr % data % libusb_error_name( rcode );
    
    return false;
}

bool
arpproxy::impl::CmdRegBitWrite( uint32_t addr, uint32_t data, uint32_t mask )
{
    uint32_t value, rcode;
    if ( (rcode = CmdRegRead( addr, value ) ) ) {
        return (rcode = CmdRegWrite( addr, (value & ~mask) | (data & mask) ) ) == 0;
    }
    log() << boost::format( "CmdRegBitWrie( 0x%x:0x%x ) %s." ) % addr % data % libusb_error_name( rcode );    
    return false;
}

bool
arpproxy::impl::CmdRegVectorWriteHelper( const uint32_t * data, size_t nitem )
{
    int transferred, rcode;

    if ( ( rcode = bulk_transfer( RegBulkOut
                                  , reinterpret_cast< const uint8_t * >( data ), int(nitem) * 12, transferred ) ) == 0 ) {
        // it looks like read operation for the number of triple-words time required
        uint8_t temp[8];
        for ( size_t a = 0; a < nitem && rcode == 0; ++a )
            rcode = bulk_transfer( RegBulkIn, temp, sizeof(temp), transferred ); // read
    } else {
        log() << boost::format( "CmdRegVectorWrite failed with code: %1%" ) % libusb_error_name( rcode );
        return false;
    }
    return rcode == 0;
}

#if !defined countof
# define countof(x) (sizeof(x)/sizeof(x[0]))
#endif

void
arpproxy::impl::handle_device_digitizer_initialize()
{
// call after fpga reset
// ics8442
#define	ARP_FPGACLKSET_OFFSET	0x00001800
#define	ARP_FPGACLKSET_XTAL_SEL				(ARP_FPGACLKSET_OFFSET + 0x00)
#define	ARP_FPGACLKSET_TEST_OUT				(ARP_FPGACLKSET_OFFSET + 0x04)
#define	ARP_FPGACLKSET_N					(ARP_FPGACLKSET_OFFSET + 0x04)
#define	ARP_FPGACLKSET_M					(ARP_FPGACLKSET_OFFSET + 0x04)

#define	ARP_ADC_CTRL_OFFSET		0x00004000
#define ARP_AD_ADC_POWER					(ARP_ADC_CTRL_OFFSET + 0x000)
#define ARP_AD_ADC_SEL						(ARP_ADC_CTRL_OFFSET + 0x004)
#define ARP_AD_E2V_MODE						(ARP_ADC_CTRL_OFFSET + 0x104)
#define ARP_AD_E2VPLL_EN					(ARP_ADC_CTRL_OFFSET + 0x110)
#define ARP_AD_E2VPLL_S_TW					(ARP_ADC_CTRL_OFFSET + 0x120)
#define ARP_AD_E2VPLL_S_REG1				(ARP_ADC_CTRL_OFFSET + 0x114)
#define ARP_AD_E2VPLL_S_REG2				(ARP_ADC_CTRL_OFFSET + 0x118)
#define ARP_AD_E2VPLL_S_REG3				(ARP_ADC_CTRL_OFFSET + 0x11C)
#define ARP_AD_SDAC							(ARP_ADC_CTRL_OFFSET + 0x280)

#define	ARP_SDAC_SPI_OFFSET	(ARP_ADC_CTRL_OFFSET + 0x200)
#define ARP_AD_SDAC_RG_QMR					(ARP_SDAC_SPI_OFFSET + 0x60)
#define ARP_AD_SADC_RG_STX0					(ARP_SADC_SPI_OFFSET + 0x00)
#define	ARP_SDAC_REFV_MAX	(2500)
#define	ARP_SDAC_REFV_MIN	(-2500)

#define ARP_AD_SDAC_RG_CMD0					(ARP_SDAC_SPI_OFFSET + 0x40)
#define ARP_AD_SDAC_RG_QWR					(ARP_SDAC_SPI_OFFSET + 0x68)
#define ARP_AD_SDAC_RG_QDLYR				(ARP_SDAC_SPI_OFFSET + 0x64)
#define ARP_AD_SDAC_RG_STX1					(ARP_SDAC_SPI_OFFSET + 0x04)
#define ARP_AD_SDAC_RG_CMD1					(ARP_SDAC_SPI_OFFSET + 0x44)
#define ARP_AD_SDAC_RG_STX2					(ARP_SDAC_SPI_OFFSET + 0x08)
#define ARP_AD_SDAC_RG_CMD2					(ARP_SDAC_SPI_OFFSET + 0x48)
#define ARP_AD_SDAC_RG_STX0					(ARP_SDAC_SPI_OFFSET + 0x00)
#define ARP_AD_SDAC_RG_STX1					(ARP_SDAC_SPI_OFFSET + 0x04)
#define ARP_AD_SDAC_RG_STX2					(ARP_SDAC_SPI_OFFSET + 0x08)
#define ARP_AD_SDAC_RG_STX3					(ARP_SDAC_SPI_OFFSET + 0x0C)
#define ARP_AD_SDAC_RG_STX4					(ARP_SDAC_SPI_OFFSET + 0x10)
#define ARP_AD_SDAC_RG_STX5					(ARP_SDAC_SPI_OFFSET + 0x14)
#define ARP_AD_SDAC_RG_STX6					(ARP_SDAC_SPI_OFFSET + 0x18)
#define ARP_AD_SDAC_RG_STX7					(ARP_SDAC_SPI_OFFSET + 0x1C)
#define ARP_AD_SDAC_RG_CMD0					(ARP_SDAC_SPI_OFFSET + 0x40)
#define ARP_AD_SDAC_RG_CMD1					(ARP_SDAC_SPI_OFFSET + 0x44)
#define ARP_AD_SDAC_RG_CMD2					(ARP_SDAC_SPI_OFFSET + 0x48)
#define ARP_AD_SDAC_RG_CMD3					(ARP_SDAC_SPI_OFFSET + 0x4C)
#define ARP_AD_SDAC_RG_CMD4					(ARP_SDAC_SPI_OFFSET + 0x50)
#define ARP_AD_SDAC_RG_CMD5					(ARP_SDAC_SPI_OFFSET + 0x54)
#define ARP_AD_SDAC_RG_CMD6					(ARP_SDAC_SPI_OFFSET + 0x58)
#define ARP_AD_SDAC_RG_CMD7					(ARP_SDAC_SPI_OFFSET + 0x5C)
#define ARP_AD_SDAC_RG_QMR					(ARP_SDAC_SPI_OFFSET + 0x60)

#define	ARP_SDAC_SPI_OFFSET	(ARP_ADC_CTRL_OFFSET + 0x200)
#define	ARP_SADC_SPI_OFFSET	(ARP_ADC_CTRL_OFFSET + 0x300)
#define ARP_AD_SADC_RG_QMR					(ARP_SADC_SPI_OFFSET + 0x60)
#define ARP_AD_SADC_RG_CMD0					(ARP_SADC_SPI_OFFSET + 0x40)
#define ARP_AD_SADC_RG_QWR					(ARP_SADC_SPI_OFFSET + 0x68)
#define ARP_AD_SADC_RG_QDLYR				(ARP_SADC_SPI_OFFSET + 0x64)
#define ARP_AD_SADC_RG_STX0					(ARP_SADC_SPI_OFFSET + 0x00)
#define ARP_AD_SADC_RG_STX1					(ARP_SADC_SPI_OFFSET + 0x04)
#define ARP_AD_SADC_RG_STX2					(ARP_SADC_SPI_OFFSET + 0x08)
#define ARP_AD_SADC_RG_STX3					(ARP_SADC_SPI_OFFSET + 0x0C)
#define ARP_AD_SADC_RG_STX4					(ARP_SADC_SPI_OFFSET + 0x10)
#define ARP_AD_SADC_RG_STX5					(ARP_SADC_SPI_OFFSET + 0x14)
#define ARP_AD_SADC_RG_STX6					(ARP_SADC_SPI_OFFSET + 0x18)
#define ARP_AD_SADC_RG_STX7					(ARP_SADC_SPI_OFFSET + 0x1C)
#define ARP_AD_SADC_RG_SRX0					(ARP_SADC_SPI_OFFSET + 0x20)
#define ARP_AD_SADC_RG_SRX1					(ARP_SADC_SPI_OFFSET + 0x24)
#define ARP_AD_SADC_RG_SRX2					(ARP_SADC_SPI_OFFSET + 0x28)
#define ARP_AD_SADC_RG_SRX3					(ARP_SADC_SPI_OFFSET + 0x2C)
#define ARP_AD_SADC_RG_SRX4					(ARP_SADC_SPI_OFFSET + 0x30)
#define ARP_AD_SADC_RG_SRX5					(ARP_SADC_SPI_OFFSET + 0x34)
#define ARP_AD_SADC_RG_SRX6					(ARP_SADC_SPI_OFFSET + 0x38)
#define ARP_AD_SADC_RG_SRX7					(ARP_SADC_SPI_OFFSET + 0x3C)
#define ARP_AD_SADC_RG_CMD0					(ARP_SADC_SPI_OFFSET + 0x40)
#define ARP_AD_SADC_RG_CMD1					(ARP_SADC_SPI_OFFSET + 0x44)
#define ARP_AD_SADC_RG_CMD2					(ARP_SADC_SPI_OFFSET + 0x48)
#define ARP_AD_SADC_RG_CMD3					(ARP_SADC_SPI_OFFSET + 0x4C)
#define ARP_AD_SADC_RG_CMD4					(ARP_SADC_SPI_OFFSET + 0x50)
#define ARP_AD_SADC_RG_CMD5					(ARP_SADC_SPI_OFFSET + 0x54)
#define ARP_AD_SADC_RG_CMD6					(ARP_SADC_SPI_OFFSET + 0x58)
#define ARP_AD_SADC_RG_CMD7					(ARP_SADC_SPI_OFFSET + 0x5C)
#define ARP_AD_SADC_RG_QMR					(ARP_SADC_SPI_OFFSET + 0x60)
#define ARP_AD_SADC_RG_QDLYR				(ARP_SADC_SPI_OFFSET + 0x64)
#define ARP_AD_SADC_RG_QWR					(ARP_SADC_SPI_OFFSET + 0x68)
    
#define	ARP_CON_DDR2_RST_RELEASE			(ARP_SYS_CTRL_OFFSET + 0x08)

#define	ARP_SYS_CTRL_OFFSET		0x00003C00
#define	ARP_CON_CLKE2V						(ARP_SYS_CTRL_OFFSET + 0x04)
#define	ARP_CON_RELEASE						(ARP_SYS_CTRL_OFFSET + 0x0C)
#define	ARP_ADC_CTRL_OFFSET		0x00004000
#define ARP_AD_E2V_RESET					(ARP_ADC_CTRL_OFFSET + 0x100)
#define ARP_AD_E2V_MODE						(ARP_ADC_CTRL_OFFSET + 0x104)
// Virtex-5 Monitor
#define	ARP_SYSMON_OFFSET		0x00003800

    // e2v I/F
#define	ARP_E2V_OFFSET				0x00005400
#define	ARP_E2V_INPUT_MODE					(ARP_E2V_OFFSET + 0x00)
#define	ARP_E2V_FIFO_CLR					(ARP_E2V_OFFSET + 0x04)
#define	ARP_E2V_OVERFLOW					(ARP_E2V_OFFSET + 0x08)
#define	ARP_E2V_IOB							(ARP_E2V_OFFSET + 0x0C)
#define	ARP_E2V_IODELAYCTRL_RDY				(ARP_E2V_OFFSET + 0x10)
#define	ARP_E2V_OFFSET_MODE					(ARP_E2V_OFFSET + 0x14)

// Dsp Main
#define	ARP_DSP_OFFSET				0x00006800
#define	ARP_DSP_INOUT_MODE					(ARP_DSP_OFFSET + 0x00)
#define	ARP_DSP_DSP_MODE					(ARP_DSP_OFFSET + 0x04)
#define	ARP_DSP_FIFO_CLR					(ARP_DSP_OFFSET + 0x08)

// PC I/F DMAC(W)
#define	ARP_PCDMAC_W_OFFSET			0x00002800
#define	ARP_PCDMAC_W_SRC_ADDR				(ARP_PCDMAC_W_OFFSET + 0x00)
#define	ARP_PCDMAC_W_DST_ADDR				(ARP_PCDMAC_W_OFFSET + 0x04)
#define	ARP_PCDMAC_W_BYTECNT				(ARP_PCDMAC_W_OFFSET + 0x08)
#define	ARP_PCDMAC_W_IRQEN					(ARP_PCDMAC_W_OFFSET + 0x0C)
#define	ARP_PCDMAC_W_START					(ARP_PCDMAC_W_OFFSET + 0x0C)
#define	ARP_PCDMAC_W_BUSY					(ARP_PCDMAC_W_OFFSET + 0x10)
#define	ARP_PCDMAC_W_DONE					(ARP_PCDMAC_W_OFFSET + 0x10)
#define	ARP_PCDMAC_W_PAUSE					(ARP_PCDMAC_W_OFFSET + 0x14)

// PC I/F DMAC(R)
#define	ARP_PCDMAC_R_OFFSET			0x00002C00
#define	ARP_PCDMAC_R_SRC_ADDR				(ARP_PCDMAC_R_OFFSET + 0x00)
#define	ARP_PCDMAC_R_DST_ADDR				(ARP_PCDMAC_R_OFFSET + 0x04)
#define	ARP_PCDMAC_R_BYTECNT				(ARP_PCDMAC_R_OFFSET + 0x08)
#define	ARP_PCDMAC_R_IRQEN					(ARP_PCDMAC_R_OFFSET + 0x0C)
#define	ARP_PCDMAC_R_START					(ARP_PCDMAC_R_OFFSET + 0x0C)
#define	ARP_PCDMAC_R_BUSY					(ARP_PCDMAC_R_OFFSET + 0x10)
#define	ARP_PCDMAC_R_DONE					(ARP_PCDMAC_R_OFFSET + 0x10)
#define	ARP_PCDMAC_R_PAUSE					(ARP_PCDMAC_R_OFFSET + 0x14)

    static Arp_TblValueDetail l_tDetail [] = {
        // iType , iAddr, iValue, iMask, iMaxLoop
        { ARP_TBLVAL_WRITE	, ARP_FPGACLKSET_M		, (0 << 12) | (0 << 9) | (10 << 0)	,0	, 0 }
        , { ARP_TBLVAL_WRITE, ARP_FPGACLKSET_XTAL_SEL , 1						, 0		, 0 }
        , { ARP_TBLVAL_WRITE, ARP_AD_ADC_POWER		, 1							, 0		, 0	}	// Power ON
        , { ARP_TBLVAL_WAIT,  0						, 1000						, 0		, 0 }
        , { ARP_TBLVAL_WRITE, ARP_AD_ADC_SEL	, 0/*2*/						, 0		, 0 }		// Not using pre-amp.
        , { ARP_TBLVAL_WRITE, ARP_AD_E2V_MODE		, 7							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_E2VPLL_EN		, 3							, 0		, 0 }		// 
        , { ARP_TBLVAL_WRITE, ARP_AD_E2VPLL_S_TW	, 0x40						, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_E2VPLL_S_REG1	, 0 /* l_tReg1.Int */		, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 5000						, 0		, 0 }
        , { ARP_TBLVAL_WRITE, ARP_AD_E2VPLL_S_REG2	, 0 /* l_tReg2.Int */		, 0		, 0 }		// 
        , { ARP_TBLVAL_WAIT	, 0						, 5000						, 0		, 0 }
        , { ARP_TBLVAL_WRITE, ARP_AD_E2VPLL_S_REG3	, 0 /* l_tReg3.Int */		, 0		, 0 }		// 

        // SDAC
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC			, 0								, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_QMR	, 0x105 /* l_tSpiRgQmr.Int */	, 0		, 0	}	// mode register
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_STX0	, 0x8000 | (int)((0-ARP_SDAC_REFV_MIN)*0x1000/(ARP_SDAC_REFV_MAX-ARP_SDAC_REFV_MIN))	, 0		, 0	}
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_CMD0	, 0								, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_QWR	, 0x1000 /* l_tSpiRgQwrGain.Int */	, 0		, 0	}	//
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_QDLYR	, 0x8000 /* 0x8000 */	, 0		, 0 }	//
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC			, 1								, 0		, 0	}	//
        , { ARP_TBLVAL_WAIT	, 0						, 100000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC			, 0								, 0		, 0	}	//
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_STX1	, 0x4000 | (int)((1200-ARP_SDAC_REFV_MIN)*0x1000/(ARP_SDAC_REFV_MAX-ARP_SDAC_REFV_MIN))		, 0		, 0	}
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_CMD1	, 0								, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_QWR	, 0x1101 /* l_tSpiRgQwrDemux.Int */		, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_QDLYR	, 0x8000 /* 0x8000 */		, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC			, 1							, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 100000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC			, 0							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_STX2	, 0x0000 | (int)((0-ARP_SDAC_REFV_MIN)*0x1000/(ARP_SDAC_REFV_MAX-ARP_SDAC_REFV_MIN))		, 0		, 0 }
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_CMD2	, 0							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_QWR	, 0x1202	, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_QDLYR	, 0x8000				, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC			, 1							, 0		, 0 }	// SDAC Load
        , { ARP_TBLVAL_WAIT	, 0						, 100000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC			, 0							, 0		, 0	}	// SDAC Normal
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_STX3	, 0xC000 | (int)((0-ARP_SDAC_REFV_MIN)*0x1000/(ARP_SDAC_REFV_MAX-ARP_SDAC_REFV_MIN))		, 0		, 0}
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_CMD3	, 0							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_QWR	, 0x1303	, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC_RG_QDLYR	, 0x8000	, 0		, 0	}	//
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC			, 1								, 0		, 0	}	// SDAC Load
        , { ARP_TBLVAL_WAIT	, 0						, 100000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SDAC			, 0								, 0		, 0	}	// SDAC Normal

        , { ARP_TBLVAL_WRITE, ARP_CON_DDR2_RST_RELEASE , 0x00						, 0		, 0	}	// DDR2 Reset
        , { ARP_TBLVAL_WRITE, ARP_CON_DDR2_RST_RELEASE , 0x02						, 0		, 0	}	// DDR2 Reset release
        , { ARP_TBLVAL_WRITE, ARP_CON_CLKE2V		, 1								, 0		, 0	}	// FPGA e2vclk PLL reset
        , { ARP_TBLVAL_WAIT	, 0						, 1000							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_CON_CLKE2V		, 0								, 0		, 0	}	// FPGA e2vclk PLL reset clear
        , { ARP_TBLVAL_COMP	, ARP_CON_CLKE2V		, 0x02							, 0x02	, 100 }	// Lock
        , { ARP_TBLVAL_WRITE, ARP_CON_RELEASE		, 0x01							, 0		, 0	}	// usb release
        , { ARP_TBLVAL_WAIT	, 0						, 1000							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_CON_RELEASE		, 0x07							, 0		, 0	}	// reset_sync reset clear

        , { ARP_TBLVAL_WRITE, ARP_AD_E2V_RESET		, 1								, 0		, 0	}	// 

        , { ARP_TBLVAL_WRITE, ARP_CON_CLKE2V		, 1								, 0		, 0	}	// FPGA e2vclk PLL reset
        , { ARP_TBLVAL_WAIT	, 0						, 1000							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_CON_CLKE2V		, 0								, 0		, 0	}	// FPGA e2vclk PLL reset clear
        , { ARP_TBLVAL_COMP	, ARP_CON_CLKE2V		, 0x02							, 0x02	, 100 }	// Lock
        , { ARP_TBLVAL_WRITE, ARP_CON_RELEASE		, 0x01							, 0		, 0	}	// usb release
        , { ARP_TBLVAL_WAIT	, 0						, 1000							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_CON_RELEASE		, 0x07							, 0		, 0	}	// reset_sync reset clear


        // FPGA System monitor 
        , { ARP_TBLVAL_WRITE, ARP_SYSMON_OFFSET + 0x120				, 0x3F00		, 0		, 0	}	// ADC channel selection Addr 48h
        , { ARP_TBLVAL_WRITE, ARP_SYSMON_OFFSET + 0x124				, 0x0400		, 0		, 0	}	// ADC channel selection Addr 49h
        , { ARP_TBLVAL_WRITE, ARP_SYSMON_OFFSET + 0x100				, 0x0000		, 0		, 0	}	// Config Reg. #1 Addr 41h 
        , { ARP_TBLVAL_WRITE, ARP_SYSMON_OFFSET + 0x104				, 0x2000		, 0		, 0	}	// Config Reg. #1 Addr 41h 

        // SADC
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QMR	, 0x1F8							, 0		, 0	}	// mode register
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_CMD0	, 0x8000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 10000							, 0		, 0 }
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QWR	, 0x1000						, 0		, 0 }	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_STX0	, 0xFFFFFFFF					, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QDLYR	, 0x8000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 10000							, 0		, 0 }
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_STX0	, 0xFFFFFFFF					, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QDLYR	, 0x8000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 10000							, 0		, 0 }
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_STX0	, 0xFFFFFFFF					, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QDLYR	, 0x8000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 10000							, 0		, 0 }
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_STX0	, 0xFFFFFFFE					, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QDLYR	, 0x8000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 10000							, 0		, 0 }
        // Configuration Ragister Write
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QMR	, 0x078							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_CMD0	, 0x0000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_STX0	, 0x03							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QDLYR	, 0x8000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 10000							, 0		, 0 }

        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QMR	, 0x1F8							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_STX0	, 0x20000000					, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QDLYR	, 0x8000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 10000							, 0		, 0 }
        , { ARP_TBLVAL_WAIT	, 0						, 20							, 0		, 0 }
        // Configuration Ragister Write
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QMR	, 0x078							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_STX0	, 0x03							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QDLYR	, 0x8000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 10000							, 0		, 0 }

        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QMR	, 0x1F8							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_STX0	, 0x00000000					, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QDLYR	, 0x8000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 10000							, 0		, 0 }
        // Configuration Ragister Read
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QMR	, 0x078							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_STX0	, 0x0B							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QDLYR	, 0x8000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 10000							, 0		, 0 }
        , { ARP_TBLVAL_WAIT	, 0						, 1000							, 0		, 0 }
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QMR	, 0x1F8							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_STX0	, 0x00000000					, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QDLYR	, 0x8000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 10000							, 0		, 0 }
        , { ARP_TBLVAL_COMP	, ARP_AD_SADC_RG_SRX0	, 0x10000000					, 0xFFFFFFFF	, 1 }
        // Cahnnel Setup Registers write
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QMR	, 0x078							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_STX0	, 0x03							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QDLYR	, 0x8000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 10000							, 0		, 0 }

        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QMR	, 0x1F8							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_STX0	, 0x02000000					, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QDLYR	, 0x8000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 10000							, 0		, 0 }

        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QMR	, 0x078							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_STX0	, 0x05							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QDLYR	, 0x8000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 10000							, 0		, 0 }
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QMR	, 0x1F8							, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_STX0	, 0x00404040					, 0		, 0	}	// 
        , { ARP_TBLVAL_WRITE, ARP_AD_SADC_RG_QDLYR	, 0x8000						, 0		, 0	}	// 
        , { ARP_TBLVAL_WAIT	, 0						, 10000							, 0		, 0 }

#ifndef ARP_HWOFFSET_DISABLE
        , { ARP_TBLVAL_WRITE, ARP_E2V_OFFSET_MODE	, (1 << 9)						, 0		, 0 }
#endif
        // buffer clear
        , { ARP_TBLVAL_WRITE, ARP_DSP_FIFO_CLR		, 1								, 0		, 0 }
        , { ARP_TBLVAL_WRITE, ARP_DSP_FIFO_CLR		, 0								, 0		, 0 }
        , { ARP_TBLVAL_WRITE, ARP_E2V_FIFO_CLR		, 1								, 0		, 0	}	// FIFO normal clear
        , { ARP_TBLVAL_WRITE, ARP_E2V_FIFO_CLR		, 0								, 0		, 0	}	// FIFO normal restore

        // Pause release
        , { ARP_TBLVAL_WRITE, ARP_PCDMAC_W_PAUSE	, 0								, 0		, 0 }
        , { ARP_TBLVAL_WRITE, ARP_PCDMAC_R_PAUSE	, 0								, 0		, 0 }
    };

    VectorInterpreter( l_tDetail, countof( l_tDetail ) );
}

void
arpproxy::impl::handle_device_fpga_reset()
{
    const uint8_t data [] = { CMD_REG_WRITE, 0x01, 0xe6, 0x80	// IFCONFIG
                              , CMD_SFR_WRITE, 0xb3, 0x00		// disable to avoid bus race condition
                              , CMD_SFR_WRITE, 0xb5, 0x00		// ibid
                              , CMD_REG_WRITE, 0xc2, 0xe6, 0xf7	// address low, high, fpga-reset
    };
    const uint8_t data2 [] = { CMD_REG_WRITE, 0xc2, 0xe6, 0xff	// address low, high, fpga-reset clear
                               , CMD_REG_WRITE, 0x01, 0xe6, 0x03	// IFCONFIG
    };
    
    int rcode(0);
    if ( ( rcode = bulk_transfer( CmdBulkOut, data ) ) == 0 ) {
        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
        rcode = bulk_transfer( CmdBulkOut, data2 );
    }

    log() << boost::format("handle_device_fpga_reset %1%") % libusb_error_name( rcode );            
}

bool
arpproxy::impl::read_device_version()
{
    int transferred, rcode;
    if ( ( rcode = bulk_transfer( CmdBulkOut, reinterpret_cast<const uint8_t *>("\x40"), 1, transferred ) ) == 0 ) {
        uint8_t xbuf[8];
        if ( ( rcode = bulk_transfer( CmdBulkIn, xbuf, sizeof(xbuf), transferred ) ) == 0 ) {
            std::string version( reinterpret_cast< const char *>(xbuf), transferred );
            ezusb_version_ = version;
            log() << boost::format( "EZUSB Ver. %1%" ) % version;
        }
    }
    return rcode == 0;
}

void
arpproxy::impl::handle_device_initialize()
{
    const uint8_t CmdUsbCtrlInit [] = { CMD_REG_WRITE
                                        , 0x01, 0xe6 /* EZUSB_REG_IFCONFIG */
                                        , 0x03
                                        , CMD_EP2468_INIT
                                        , CMD_SFR_WRITE, 0xb1, 0x00
                                        , CMD_SFR_WRITE, 0xb6, 0x00
    };
    const uint8_t CmdUsbCtrlInit2 [] = { CMD_SFR_WRITE, 0xb1, 0x40, CMD_SFR_WRITE, 0xb6, 0x40 };
    
    timer_.cancel();
    tick_ = 0;

    int rcode;
    if ( ( rcode = bulk_transfer( CmdBulkOut, CmdUsbCtrlInit ) ) == 0 ) {
        
        if ( (rcode = bulk_transfer( CmdBulkOut, CmdUsbCtrlInit2 ) ) == 0 ) {
            
            handle_device_fpga_reset();
            
            timer_.expires_from_now( std::chrono::milliseconds( 1200 ) );
            timer_.async_wait( [this]( const boost::system::error_code& ec ){ on_timer(ec); } );    
        }
    }
    
    log() << boost::format("handle_device_initialize: %1%") % libusb_error_name( rcode );
}

void
arpproxy::impl::handle_device_read_actuals()
{
    uint32_t addr = 0;
    for ( auto& a: actuals_data_ )
        CmdRegRead( ARP_OFFSET_HV + 4 * addr++, a );
    
    actuals_handler_( tick_ );
}

void
arpproxy::impl::handle_device_setvoltage( uint32_t addr, double world_value )
{
    auto it = setpt_handler::find( addr );
    if ( ! setpt_handler::empty( it ) ) {
        
        uint32_t device_value = it->second.device_value( world_value );
        setpts_data_[ addr ] = std::make_pair( world_value, device_value );

        CmdRegWrite( ARP_OFFSET_HV + addr * 4, device_value );

        if ( __verbose_level__ >= log::INFO )
            log() << boost::format( "handle_device_setvoltage( 0x%x, %d ) <= %.2f" ) % addr % device_value % world_value;

        std::string json = ( boost::format( "{ \"setpts\": [ { \"id\": \"%s\", \"value\": \"%.2f\" } ] }" ) % setpt_handler::id( it ) % world_value ).str();
        log() << json;
        notification_handler_( json );        
        
    } else {

        log() << boost::format( "error in handle_device_setvoltage( 0x%x, %.2lf )" ) % addr % world_value;

    }
}

void
arpproxy::impl::handle_device_on( bool on )
{
    setpts_data_[ arp::setpt_pumpValveCtrl ].second |= 0x8000 | 0x4000 | 0x80 | 0x1f; // Analyzer vacuum, I.S. Vacuum, guage, TMP, DFP all on
    setpts_data_[ arp::setpt_pumpValveCtrl ].second &= ~0x2000; // vent valve to be closed

    auto addr = arp::setpt_pumpValveCtrl;
    CmdRegWrite( ARP_OFFSET_HV + addr * 4, setpts_data_[ addr ].second );

    handle_device_setflag( arp::setpt_aux1, -1, on );
}

void
arpproxy::impl::handle_device_setflag( uint32_t addr, uint32_t mask, bool value )
{
    std::ostringstream json;

    uint32_t prev = setpts_data_[ addr ].second;
    
    if ( addr == arp::setpt_aux1 ) { // moduel on/off

        if ( mask == ( -1 ) ) {
            if ( value ) {
                setpts_data_[ arp::setpt_aux1 ].second = 0x1fff;
                setpts_data_[ arp::setpt_aux2 ].second |= 0x001e;
            } else {
                setpts_data_[ arp::setpt_aux1 ].second = 0x0001;
                setpts_data_[ arp::setpt_aux2 ].second &= ~0x001e;
            }
            CmdRegWrite( ARP_OFFSET_HV + arp::setpt_aux1 * 4, setpts_data_[ arp::setpt_aux1 ].second );
            CmdRegWrite( ARP_OFFSET_HV + arp::setpt_aux2 * 4, setpts_data_[ arp::setpt_aux2 ].second );
        } else {
            uint32_t f = value ? mask : 0;
            setpts_data_[ addr ].second = setpts_data_[ addr ].second & ~mask | f;
            CmdRegWrite( ARP_OFFSET_HV + addr * 4, setpts_data_[ addr ].second );
        }

    } else if ( addr == arp::setpt_aux2 ) { // Filament selection

        uint32_t f = value ? mask : 0;
        setpts_data_[ arp::setpt_aux2 ].second = setpts_data_[ arp::setpt_aux2 ].second & ~mask | f; 
        CmdRegWrite( ARP_OFFSET_HV + addr * 4, setpts_data_[ addr ].second );       

    } else if ( addr == infitof::arp::setpt_pumpValveCtrl ) {

        setpts_data_[ addr ].second |= 0x8000 | 0x4000 | 0x80 | 0x1f; // Analyzer vacuum, I.S. Vacuum, guage, TMP, DFP all on
        setpts_data_[ addr ].second &= ~0x2000; // vent valve to be closed

        uint32_t f = value ? mask : 0;
        setpts_data_[ addr ].second = setpts_data_[ addr ].second & ~mask | f;
        CmdRegWrite( ARP_OFFSET_HV + addr * 4, setpts_data_[ addr ].second );
    }

    // check if flag changed.  this prevent event fire loop between bootstrap-toggle
    if ( prev != setpts_data_[ addr ].second ) {
        int n = 0;
        json << "{ \"checkbox\": [ ";
        std::for_each( setpt_handler::begin(), setpt_handler::end(), [&]( const setpt_handler::value_type& x ){
                if ( x.second.addr == addr && x.second.mask == mask ) {
                    if ( n++ )
                        json << ", ";
                    json << boost::format( "{ \"id\": \"%s\", \"value\": \"%s\" }" ) % x.first % x.second.to_string( setpts_data_[ addr ] );
                }
            });
        json << " ]}";
        
        if ( n ) {
            log() << json.str();
            notification_handler_( json.str() );
        }

        // Text status
        std::ostringstream o;
        o << "{ \"notify\": [{ \"id\": \"status\", \"value\": \"";
        make_actual_status( o );        
        o << "\" } ]}";
        
        notification_handler_( o.str() );
    }
}

void
arpproxy::impl::handle_device_setvoltages()
{
//    for ( uint32_t addr = 0; addr < setpts_data_.size(); ++ addr )
//        handle_device_setvoltage( addr, setpts_data_[addr].second );
}

int
arpproxy::impl::bulk_transfer( endpoint ep, uint8_t * data, int length, int& transferred, uint32_t timeout )
{
    if ( usb_device_handle_ ) {
        return libusb_bulk_transfer( usb_device_handle_, ep, data, length, &transferred, timeout );
    }
    return -1;
}

int
arpproxy::impl::bulk_transfer( endpoint ep, const uint8_t * data, int length, int& transferred, uint32_t timeout )
{
    if ( usb_device_handle_ ) {
        return libusb_bulk_transfer( usb_device_handle_, ep, const_cast< uint8_t *>( data ), length, &transferred, timeout );
    }
    return -1;
}

void
arpproxy::impl::device_setvoltage( const std::string& id, double value )
{
    auto it = setpt_handler::find( id );
    if ( !setpt_handler::empty( it ) ) {
        io_service_.post( [=] { handle_device_setvoltage( it->second.addr, value ); } );
    } else {
        log() << boost::format( "unknown voltage: %1% %2%" ) % id % value;
    }
}

void
arpproxy::impl::device_setflag( const std::string& id, bool value )
{
    auto it = setpt_handler::find( id );
    if ( ! setpt_handler::empty( it ) ) {
        io_service_.post( [=] { handle_device_setflag( it->second.addr, it->second.mask, value ); } );
    } else {
        log() << boost::format( "unknown flag: %1% %2%" ) % id % value;        
    }
}

void
arpproxy::impl::make_actual_status( std::ostream& o )
{
#if defined WIN32
# pragma warning(disable:4800)
#endif
    class textcolor {
        const char * true_color_;
        const char * false_color_;
    public:
        textcolor( const char * true_color = "blue", const char * false_color = "red" )
            : true_color_( true_color ), false_color_( false_color )
            {}
        
        std::string operator()( bool state, const std::string& true_text, const std::string& false_text ) const {
            return (boost::format( "<font color='%s'>%s</font>" ) % (state ? true_color_ : false_color_) % (state ? true_text : false_text)).str();
        }
    };
    
    textcolor textile;

    uint32_t ion_vac_state = ( actuals_data_[ arp::act_pumpValveCtrl ] & 0xf0) >> 4;
    uint32_t analyzer_vac_state = actuals_data_[ arp::act_pumpValveCtrl ] & 0x0f;

    o << boost::format( "<b>I.S.:</b> VAC[%s] TMP[%s] DFP[%s] ST[0x%x]" )
        % textile( actuals_data_[ arp::act_pumpValveCtrl ] & 0x800, "RDY", "!RDY" )
        % textile( actuals_data_[ arp::act_devStateMonitor ] & 0x20, "ON", "OFF" )
        % textile( actuals_data_[ arp::act_devStateMonitor ] & 0x80, "ON", "OFF" )
        % ion_vac_state
        << boost::format( " <b>Analyzer:</b> VAC[%s] TMP[%s] DFP[%s] ST[0x%x]" )
        % textile( actuals_data_[ arp::act_pumpValveCtrl ] & 0x400, "RDY", "!RDY" )
        % textile( actuals_data_[ arp::act_devStateMonitor ] & 0x10, "ON", "OFF" )
        % textile( actuals_data_[ arp::act_devStateMonitor ] & 0x40, "ON", "OFF" )
        % analyzer_vac_state;
    o << boost::format( " <b>ALARM</b>[0x%x, 0x%x]" ) % actuals_data_[ arp::act_aux1_alarm ] % actuals_data_[ arp::act_aux2_alarm ];
    o << boost::format( " DOOR=[%s]" ) % textile(!(actuals_data_[ arp::act_devStateMonitor ] & 0x800), "CLOSE", "OPEN");
    // o << boost::format( " [%s]" ) % ((actuals_data_[ arp::act_devStateMonitor ] & 0x400) ? "LOCAL" : "REMOTE");
    o << boost::format( " GAUGE[%s]" ) % textile( actuals_data_[ arp::act_devStateMonitor ] & 0x100, "ON", "OFF");
    o << boost::format( " VENT[%s] STD.V[%s] SAMP.V[%s] IS.V[%s]" )
        % textile( !(actuals_data_[ arp::act_devStateMonitor ] & 0x08), "CLOSE", "OPEN" )  // vent
        % textile( !(actuals_data_[ arp::act_devStateMonitor ] & 0x04), "CLOSE", "OPEN" )  // std
        % textile( !(actuals_data_[ arp::act_devStateMonitor ] & 0x02), "CLOSE", "OPEN" )  // samp
        % textile( !(actuals_data_[ arp::act_devStateMonitor ] & 0x01), "CLOSE", "OPEN" ); // IS
}
