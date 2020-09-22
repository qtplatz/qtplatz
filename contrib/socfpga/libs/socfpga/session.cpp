/**************************************************************************
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "session.hpp"
#include "traceobserver.hpp"
#include <adacquire/masterobserver.hpp>
#include <adacquire/receiver.hpp>
#include <adacquire/sampleprocessor.hpp>
#include <adacquire/samplesequence.hpp>
#include <adacquire/task.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adlog/logger.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <adurl/ajax.hpp>
#include <adurl/sse.hpp>
#include <QByteArray>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <boost/asio.hpp>
#include <boost/exception/all.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <atomic>
#include <bitset>
#include <chrono>
#include <future>
#include <memory>
#include <sstream>
#include <string>

namespace socfpga {

    namespace dgmod {

        struct session::impl {

            impl() : work_( io_service_ )
                   , masterObserver_( std::make_shared< adacquire::MasterObserver >( "acquire.master.observer.ms-cheminfo.com" ) )
                   , traceObserver_( std::make_shared< socfpga::dgmod::TraceObserver >() )
                   , sse_( std::make_unique< adurl::sse_handler >( io_service_ ) )
                   , pos_( 0 )
                   , acquisition_active_( false )
                   , software_inject_requested_( false )
                   , software_inject_posix_time_( 0 ) {
                masterObserver_->addSibling( traceObserver_.get() );
            }

            static std::shared_ptr< session > instance_;

            std::mutex mutex_;

            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;
            std::vector< std::thread > threads_;

            typedef std::pair< std::shared_ptr< adacquire::Receiver >, std::string > client_pair_t;
            std::vector< client_pair_t > clients_;

            std::shared_ptr< adacquire::MasterObserver > masterObserver_;
            std::shared_ptr< TraceObserver > traceObserver_;

            std::pair< std::string, std::string > httpd_address_;
            std::unique_ptr< adurl::sse_handler > sse_;
            std::uint32_t pos_;
            bool acquisition_active_;
            bool software_inject_requested_;
            uint64_t software_inject_posix_time_;

            void reply_message( adacquire::Receiver::eINSTEVENT msg, uint32_t value ) {
                std::lock_guard< std::mutex > lock( mutex_ );
                for ( auto& r: clients_ )
                    r.first->message( msg, value );
            }

            void reply_handler( const std::string& method, const std::string& reply) {
                if ( method == "InitialSetup" ) {
                    reply_message( adacquire::Receiver::STATE_CHANGED
                                   , ( reply == "success" )
                                   ? adacquire::Instrument::eStandBy
                                   : adacquire::Instrument::eNotConnected | adacquire::Instrument::eErrorFlag );
                } else if ( method == "StateChanged" ) {
                    if ( reply == "Stopped" )
                        reply_message( adacquire::Receiver::STATE_CHANGED, adacquire::Instrument::eStop ); // 9
                    else if ( reply == "Running" )
                        reply_message( adacquire::Receiver::STATE_CHANGED, adacquire::Instrument::eRunning ); // 8
                } else if ( method == "DarkStarted" ) {
                    reply_message( adacquire::Receiver::DARK_STARTED, 1 );
                } else if ( method == "DarkCanceled" ) {
                    reply_message( adacquire::Receiver::DARK_CANCELED, 0 );
                } else if ( method == "DarkAcquired" ) {
                    reply_message( adacquire::Receiver::DARK_ACQUIRED, 0 );
                } else {
                    ADINFO() << "ACQUIRE: " << method << " = " << reply;
                }
            }

            void reply_info( const std::string& json ) {
                for ( auto& r: clients_ )
                    r.first->notify_info( json );
            }

            void connect_sse( const std::string& host, const std::string& port, const std::string& url );
        };
    }
}

using namespace socfpga::dgmod;

session::session() : impl_( new impl() )
{
}

session::~session()
{
    delete impl_;
}

std::string
session::software_revision() const
{
    return "3.2";
}

bool
session::setConfiguration( const std::string& json )
{
    boost::property_tree::ptree pt;
    std::istringstream in( json );
    boost::property_tree::read_json( in, pt );

    if ( auto ip_addr = pt.get_optional< std::string >( "ip_address" ) ) {
        if ( auto port = pt.get_optional< std::string >( "port" ) ) {
            impl_->httpd_address_ = std::make_pair( ip_addr.get(), port.get() );
            impl_->connect_sse( ip_addr.get(), port.get(), "/ad/api$events" );
        }
    }
    return true;
}

const char *
session::configuration() const
{
    return nullptr;
}

bool
session::configComplete()
{
    return true;
}

bool
session::connect( adacquire::Receiver * receiver, const std::string& token )
{
    auto ptr( receiver->shared_from_this() );

#if ! defined NDEBUG
    ADDEBUG() << __FUNCTION__ << " token: " << token;
#endif

    if ( ptr ) {
        impl_->clients_.emplace_back( ptr, token );

        impl_->io_service_.post(
            [this] () { impl_->reply_message( adacquire::Receiver::CLIENT_ATTACHED, uint32_t( impl_->clients_.size() ) );
            });
        return true;
    }
    return false;
}

bool
session::disconnect( adacquire::Receiver * receiver )
{
    auto self( receiver->shared_from_this() );

    std::lock_guard< std::mutex > lock( impl_->mutex_ );
    auto it = std::find_if( impl_->clients_.begin(), impl_->clients_.end(), [self]( const impl::client_pair_t& a ){
            return a.first == self; });

    if ( it != impl_->clients_.end() ) {
        impl_->clients_.erase( it );
        return true;
    }

    return false;
}

uint32_t
session::get_status()
{
    return 0;
}

adacquire::SignalObserver::Observer *
session::getObserver()
{
    return impl_->masterObserver_.get();
}

bool
session::initialize()
{
    return true;
}

bool
session::shutdown()
{
    impl_->io_service_.stop();

    for ( auto& t: impl_->threads_ )
        t.join();

    return true;
}

bool
session::echo( const std::string& msg )
{
    return false;
}

bool
session::shell( const std::string& cmdline )
{
    return false;
}

std::shared_ptr< const adcontrols::ControlMethod::Method >
session::getControlMethod()
{
    return nullptr; // adacquire::ControlMethod::Method();
}

bool
session::prepare_for_run( std::shared_ptr< const adcontrols::ControlMethod::Method > )
{
    impl_->software_inject_requested_ = false;
    impl_->acquisition_active_ = false;
    impl_->software_inject_posix_time_ = 0;
    return false;
}

bool
session::prepare_for_run( const std::string& json, arg_type atype )
{
    impl_->software_inject_requested_ = false;
    impl_->acquisition_active_ = false;
    impl_->software_inject_posix_time_ = 0;
    if ( atype != arg_json )
        return false;
    return true;
}

bool
session::event_out( uint32_t event )
{
    if ( event == adacquire::Instrument::instEventInjectOut ) { // software inject
        if ( !impl_->acquisition_active_ ) {
            adurl::ajax ajax( impl_->httpd_address_.first, impl_->httpd_address_.second );
            if ( ajax( "POST", "/ad/api$event_out", "{\"flags\":fake_inject}", "application/json" ) ) {
                size_t size(0);
            }
        }
    }
    return true;
}

bool
session::start_run()
{
    return true; // impl_->digitizer_->peripheral_run();
}

bool
session::suspend_run()
{
    return true;
}

bool
session::resume_run()
{
    return true;
}

bool
session::stop_run()
{
    return true; // impl_->digitizer_->peripheral_stop();
}

bool
session::time_event_trigger( std::shared_ptr< const adcontrols::ControlMethod::TimedEvents > tt
                             , adcontrols::ControlMethod::const_time_event_iterator begin
                             , adcontrols::ControlMethod::const_time_event_iterator end )
{
    return true;
}


bool
session::dark_run( size_t waitCount )
{
    return true; // impl_->digitizer_->peripheral_dark( waitCount );
}

void
session::impl::connect_sse( const std::string& host, const std::string& port, const std::string& url )
{
#if ! defined NDEBUG
    ADDEBUG() << __FUNCTION__ << " " << host << ":" << port << "/" << url;
#endif

    sse_->connect(
        url
        , host
        , port
        , [&]( adurl::sse_event_data_t&& ev ) {

            std::string event, data;
            std::tie( event, std::ignore, data ) = std::move( ev );


            if ( event == "ad.values" ) {

                reply_info( data );

                auto jdoc = QJsonDocument::fromJson( QByteArray( data.data(), data.size() ) );

                if ( jdoc.object().contains( "ad.values" ) ) {

                    auto jarray = jdoc.object()[ "ad.values" ].toArray();
                    std::lock_guard< std::mutex > lock( mutex_ );
                    uint32_t events(0);
                    std::vector< advalue > values;

                    for ( const auto& jadval: jarray ) {
#if ! defined NDEBUG && 0
                        qDebug() << jadval;
#endif
                        advalue value;
                        auto jobj = jadval.toObject();
                        value.posix_time   = jobj[ "posix_time" ].toString().toULongLong(); // SoC kernel time (must be using ptpd)
                        value.adc_counter  = jobj[ "adc_c" ].toString().toULong();        // ADC trigger counter (250kHz)
                        value.elapsed_time = jobj[ "elapsed" ].toString().toULongLong();  // fpga clock in ns
                        value.flags_time   = jobj[ "flags_t" ].toString().toULongLong();
                        value.nacc         = jobj[ "nacc" ].toInt();

                        auto jflags = jobj[ "flags" ].toArray();
                        std::array< uint32_t, 2 > flags;
                        std::transform( jflags.begin(), jflags.end(), flags.begin()
                                        , [&](const auto& o){ return o.toString().toUInt(); } );
                        value.flags = flags[ 0 ];

                        events |= value.flags;

                        auto vec = jobj[ "ad" ].toArray();
                        std::transform( vec.begin(), vec.end(), value.ad.begin()
                                        , [&](const auto& o ){ return double(o.toInt()) / value.nacc; });

                        // handle hardware injection -- this is desiered
                        if ( value.flags & adacquire::SignalObserver::wkEvent_INJECT ) {
                            if ( !acquisition_active_ )
                                traceObserver_->setInjectData( value );
                            else
                                value.flags &= ~adacquire::SignalObserver::wkEvent_INJECT;
                            acquisition_active_ = true; // hardware inject event received.
                        }

                        // handle software injection timing -- this is workaround
                        if ( !acquisition_active_ && software_inject_requested_ ) {
                            value.flags |= adacquire::SignalObserver::wkEvent_INJECT;  // force set inject event received.
                            value.flags_time = software_inject_posix_time_;
                            acquisition_active_ = true;
                            software_inject_requested_ = false;
                        }

                        values.emplace_back( value );
                    }
                    auto pos = traceObserver_->emplace_back( std::move( values ), events );
                    masterObserver_->dataChanged( traceObserver_.get(), pos );
                }

            }
        } );

    // sse_->connect( url, host, port );
    threads_.emplace_back( [&]{ io_service_.run(); } );
}
