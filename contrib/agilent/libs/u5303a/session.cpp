/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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
#include "waveformobserver.hpp"
#include <u5303a/digitizer.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adacquire/masterobserver.hpp>
#include <adacquire/receiver.hpp>
#include <adlog/logger.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/utf.hpp>
#include <adportable/debug.hpp>
#include <adportable/semaphore.hpp>
#include <adcontrols/controlmethod/timedevent.hpp>
#include <adcontrols/controlmethod/timedevents.hpp>
#include <socfpga/constants.hpp>
#include <boost/asio.hpp>
#include <boost/exception/all.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <atomic>
#include <future>
#include <memory>
#include <sstream>

namespace adi = adacquire;

namespace u5303a { namespace Instrument {

        struct Session::impl {

            impl() : work_( io_service_ )
                   , masterObserver_( std::make_shared< adacquire::MasterObserver >( "u5303a.master.observer.ms-cheminfo.com" ) )
                   , waveformObserver_( std::make_shared< WaveformObserver >() ) {

                masterObserver_->addSibling( waveformObserver_.get() );

            }

            static std::once_flag flag_, flag2_, flag3_;
            static std::shared_ptr< Session > instance_;
            static std::mutex mutex_;

            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;
            std::vector< std::thread > threads_;

            typedef std::pair< std::shared_ptr< adi::Receiver >, std::string > client_pair_t;
            std::vector< client_pair_t > clients_;
            inline std::mutex& mutex() { return mutex_; }

            std::shared_ptr< u5303a::digitizer > digitizer_;
            std::shared_ptr< adacquire::MasterObserver > masterObserver_;
            std::shared_ptr< WaveformObserver > waveformObserver_;

            void reply_message( adi::Receiver::eINSTEVENT msg, uint32_t value ) {
                std::lock_guard< std::mutex > lock( mutex_ );
                for ( auto& r: clients_ )
                    r.first->message( msg, value );
            }

            void reply_handler( const std::string& method, const std::string& reply) {
                if ( method == "InitialSetup" ) {
                    reply_message( adi::Receiver::STATE_CHANGED
                                   , ( reply == "success" ) ? adi::Instrument::eStandBy : adi::Instrument::eNotConnected | adi::Instrument::eErrorFlag );
                } else if ( method == "StateChanged" ) {
                    if ( reply == "Stopped" )
                        reply_message( adi::Receiver::STATE_CHANGED, adi::Instrument::eStop ); // 9
                    else if ( reply == "Running" )
                        reply_message( adi::Receiver::STATE_CHANGED, adi::Instrument::eRunning ); // 8
                } else if ( method == "DarkStarted" ) {
                    reply_message( adi::Receiver::DARK_STARTED, 1 );
                } else if ( method == "DarkCanceled" ) {
                    reply_message( adi::Receiver::DARK_CANCELED, 0 );
                } else if ( method == "DarkAcquired" ) {
                    reply_message( adi::Receiver::DARK_ACQUIRED, 0 );
                } else {
                    ADINFO() << "U5303A: " << method << " = " << reply;
                }
            }

            bool waveform_handler( const acqrscontrols::u5303a::waveform * ch1
                                   , const acqrscontrols::u5303a::waveform * ch2
                                   , acqrscontrols::u5303a::method& next ) {
                if ( masterObserver_ && waveformObserver_ ) {
                    if ( ch1 || ch2 ) {
                        auto pair = std::make_pair( ( ch1 ? ch1->shared_from_this() : 0 ), ( ch2 ? ch2->shared_from_this() : 0 ) );
                        auto pos = (*waveformObserver_) << pair;
                        masterObserver_->dataChanged( waveformObserver_.get(), pos );
                        return false; // no next method changed.
                    }
                }
                return false;
            }
        };

        std::once_flag Session::impl::flag_;
        std::once_flag Session::impl::flag2_;
        std::once_flag Session::impl::flag3_;
        std::mutex Session::impl::mutex_;
        std::shared_ptr< Session > Session::impl::instance_;

    }
}

using namespace u5303a::Instrument;

Session *
Session::instance()
{
    std::call_once( impl::flag_, [&] () { impl::instance_ = std::make_shared< Session >(); } );
    return impl::instance_.get();
}

Session::Session() : impl_( new impl() )
{
}

Session::~Session()
{
    delete impl_;
}

std::string
Session::software_revision() const
{
    return "3.2";
}

bool
Session::setConfiguration( const std::string& xml )
{
    return true;
}

bool
Session::configComplete()
{
    return true;
}

bool
Session::connect( adi::Receiver * receiver, const std::string& token )
{
    auto ptr( receiver->shared_from_this() );

    if ( ptr ) {
        std::call_once( impl::flag2_, [&] () {
                impl_->threads_.push_back( adportable::asio::thread( [=]() {
                    try {
                        impl_->io_service_.run();
                    } catch ( std::exception& ex ) {
                        ADDEBUG() << boost::current_exception_diagnostic_information();
                        BOOST_THROW_EXCEPTION( ex );
                    }
                    } ) );
            });

        do {
            std::lock_guard< std::mutex > lock( impl_->mutex() );
            impl_->clients_.emplace_back( ptr, token );
        } while ( 0 );

        impl_->io_service_.post( [this] () { impl_->reply_message( adi::Receiver::CLIENT_ATTACHED, uint32_t( impl_->clients_.size() ) ); } );

        return true;
    }
    return false;
}

bool
Session::disconnect( adacquire::Receiver * receiver )
{
    auto self( receiver->shared_from_this() );

    std::lock_guard< std::mutex > lock( impl_->mutex() );
    auto it = std::find_if( impl_->clients_.begin(), impl_->clients_.end(), [self]( const impl::client_pair_t& a ){
            return a.first == self; });

    if ( it != impl_->clients_.end() ) {
        impl_->clients_.erase( it );
        return true;
    }

    return false;
}

uint32_t
Session::get_status()
{
    return 0;
}

adacquire::SignalObserver::Observer *
Session::getObserver()
{
    return impl_->masterObserver_.get();
}

bool
Session::initialize()
{
    std::call_once( impl::flag3_, [&] () {
            std::lock_guard< std::mutex > lock( impl::mutex_ );
            impl_->digitizer_ = std::make_shared< u5303a::digitizer >();
            impl_->digitizer_->connect_reply( std::bind( &impl::reply_handler, impl_
                                                         , std::placeholders::_1
                                                         , std::placeholders::_2 ) );
            impl_->digitizer_->connect_waveform( std::bind( &impl::waveform_handler, impl_
                                                            , std::placeholders::_1
                                                            , std::placeholders::_2
                                                            , std::placeholders::_3 ) );
        } );
    try {
        return impl_->digitizer_->peripheral_initialize();
    } catch ( std::exception& ex ) {
        ADDEBUG() << boost::current_exception_diagnostic_information();
        BOOST_THROW_EXCEPTION( ex );
    }
}

bool
Session::shutdown()
{
    impl_->digitizer_ && impl_->digitizer_->peripheral_terminate();

    impl_->io_service_.stop();

    for ( auto& t : impl_->threads_ )
        t.join();

    return true;
}

bool
Session::echo( const std::string& msg )
{
    return false;
}

bool
Session::shell( const std::string& cmdline )
{
    return false;
}

std::shared_ptr< const adcontrols::ControlMethod::Method >
Session::getControlMethod()
{
    return 0; // adacquire::ControlMethod::Method();
}

bool
Session::prepare_for_run( std::shared_ptr< const adcontrols::ControlMethod::Method > m )
{
    if ( m ) {
        auto it = m->find( m->begin(), m->end(), acqrscontrols::u5303a::method::clsid() );
        if ( it != m->end() ) {
            acqrscontrols::u5303a::method method;
            if ( it->get<>( *it, method ) ) {
                try {
                    return impl_->digitizer_->peripheral_prepare_for_run( method );
                } catch ( std::exception& ex ) {
                    ADDEBUG() << boost::current_exception_diagnostic_information();
                    BOOST_THROW_EXCEPTION( ex );
                }
            }
        }
    }
    return false;
}

bool
Session::event_out( uint32_t event )
{
#if !defined NDEBUG && 0
    ADDEBUG() << "##### Session::event_out( " << event << " )";
#endif
    return impl_->digitizer_->peripheral_trigger_inject();
}

bool
Session::start_run()
{
    return impl_->digitizer_->peripheral_run();
}

bool
Session::suspend_run()
{
    return true;
}

bool
Session::resume_run()
{
    return true;
}

bool
Session::stop_run()
{
    return impl_->digitizer_->peripheral_stop();
}

bool
Session::time_event_trigger( std::shared_ptr< const adcontrols::ControlMethod::TimedEvents > tt
                             , adcontrols::ControlMethod::const_time_event_iterator begin
                             , adcontrols::ControlMethod::const_time_event_iterator end )
{
    std::for_each( begin, end, [&]( const auto& e ){
        if ( e.modelClsid() == socfpga::infitof::dgmod_protocol && e.data_type() == "application/json" ) {
            // see infitof/src/plugins/infitof2/document.cpp
            if ( auto pt = e.ptree()->template get_child_optional( "data.value" ) ) {
                auto method = impl_->digitizer_->method();
                if ( method.import( *pt ) ) {
                    ADDEBUG() << "------------------------------>\n" <<
                        method.toJson();
                    try {
                        return impl_->digitizer_->peripheral_prepare_for_run( method );
                    } catch ( std::exception& ) {
                        ADDEBUG() << boost::current_exception_diagnostic_information();
                    }
                }
            }
        }
    });
    ADDEBUG() << "<------------------------------";
    return true;
}


bool
Session::dark_run( size_t waitCount )
{
    return impl_->digitizer_->peripheral_dark( waitCount );
}
