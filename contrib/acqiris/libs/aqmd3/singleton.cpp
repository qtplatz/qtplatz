/**************************************************************************
** Copyright (C) 2018 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "condition_wait.hpp"
#include "constants.hpp"
#include "digitizer.hpp"
#include "singleton.hpp"
#include "waveformobserver.hpp"
#include <aqmd3controls/meta_data.hpp>
#include <aqmd3controls/waveform.hpp>
#include <adportable/semaphore.hpp>
#include <adacquire/receiver.hpp>
#include <adacquire/masterobserver.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <boost/format.hpp>

using namespace aqmd3;
using namespace std::literals::chrono_literals;

using adportable::semaphore;

static uint16_t datamover_tag;

singleton::singleton() : sema_( std::make_unique< semaphore >() )
                       , sequence_number_( 0x100 )
                       , masterObserver_( std::make_shared< adacquire::MasterObserver >( "aqmd3.master.observer.ms-cheminfo.com" ) )
                       , waveformObserver_( std::make_shared< WaveformObserver >() )
                       , clocks_( 0 )
                       , triggers_( 0 )
                       , adc_core_clock_( 0 )
                       , uptime_( std::chrono::system_clock::now() )
                       , pos_( 0 )
                       , inject_trigger_latch_( false )
                       , inject_trigger_latch_count_( 0 )
                       , hvdg_json_( "{}" )
                       , digitizer_( std::make_unique< aqmd3::digitizer >() )
{
    masterObserver_->addSibling( waveformObserver_.get() );
    using namespace std::placeholders;
    digitizer_->connect_reply( std::bind( &singleton::reply_handler, this, _1, _2 ) );
    digitizer_->connect_waveform( std::bind( &singleton::waveform_handler, this, _1, _2, _3 ) );
}

singleton::~singleton()
{
    //ADDEBUG() << "##### singleton dtor #####";
}

singleton *
singleton::instance()
{
    static singleton __instance;
    return &__instance;
}

bool
singleton::finalize()
{
    std::lock_guard< std::mutex > lock( mutex_ );

    // for ( auto c: connections_ )
    //     c->shutdown();
    io_service_.stop();
    for ( auto& t: threads_ )
        t.join();

    connections_.clear();

    ADDEBUG() << "##### singleton finalize gracefully #####";
}

bool
singleton::initialize()
{
    std::lock_guard< std::mutex > lock( mutex_ );

    static std::once_flag flag;
    std::call_once( flag, [&]{ threads_.emplace_back( std::thread( [&]{ io_service_.run(); } ) ); } );
    return true;
}

bool
singleton::is_open() const
{
    return ! threads_.empty();
}

void
singleton::enqueue( std::string&& data )
{
    {
        std::lock_guard< std::mutex > lock( mutex_ );
        queue_.emplace( data );
    }
    sema_->signal();
}

bool
singleton::dequeue( std::string& data )
{
    sema_->wait();
    std::lock_guard< std::mutex > lock( mutex_ );
    data = std::move( queue_.front() );
    queue_.pop();
    return true;
}

bool
singleton::dequeue( std::string& data, const std::chrono::milliseconds& duration )
{
    if ( sema_->wait( duration ) ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        data = std::move( queue_.front() );
        queue_.pop();
        return true;
    }
    return false;
}

uint16_t
singleton::get_sequence_number()
{
    return sequence_number_++;
}

bool
singleton::connect( std::shared_ptr< adacquire::Receiver > ptr, const std::string& token )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    auto it = std::find( connections_.begin(), connections_.end(), ptr );
    if ( it == connections_.end() ) {
        connections_.emplace_back( ptr );
        reply_message( adacquire::Receiver::CLIENT_ATTACHED, connections_.size() );
    }
    return true;
}

bool
singleton::disconnect( std::shared_ptr< adacquire::Receiver > ptr )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    auto it = std::remove( connections_.begin(), connections_.end(), ptr );
    if ( it != connections_.end() )
        connections_.erase( it );
    return true;
}

adacquire::SignalObserver::Observer *
singleton::getObserver()
{
    return masterObserver_.get();
}

bool
singleton::post( std::pair< std::shared_ptr< const waveform >, std::shared_ptr< const waveform > >&& avgpkd )
{
    // if ( masterObserver_ && waveformObserver_ ) {
    //     auto pos = avgpkd.first->pos();
    //     //waveformObserver_->emplace_back( std::move( avgpkd ) );
    //     masterObserver_->dataChanged( waveformObserver_.get(), pos );
    // }
    return true;
    //return handle_waveform( std::move( avgpkd.first ) );
}

void
singleton::set_inject_trigger_in( bool set )
{
    inject_trigger_latch_ = set;
    if ( set )
        inject_trigger_latch_count_ = 5;
}

bool
singleton::inject_trigger() const
{
    return inject_trigger_latch_;
}

void
singleton::set_hvdg_status( std::string&& json )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    hvdg_json_ = std::move( json );
}

void
singleton::reply_message( int msg, int value )
{
    io_service_.post( [=](){
        std::lock_guard< std::mutex > lock( mutex_ );
            for ( auto& receiver: connections_ )
                receiver->message( adacquire::Receiver::eINSTEVENT( msg ), value );
        });
}

///////
void
singleton::reply_handler( const std::string& method, const std::string& reply )
{
    // ADDEBUG() << "## " << __FUNCTION__ << "(" << method << ", " << reply << ")";
    ADINFO() << "## " << __FUNCTION__ << "(" << method << ", " << reply << ")";
}

bool
singleton::waveform_handler( const aqmd3controls::waveform * ch1, const aqmd3controls::waveform * ch2, aqmd3controls::method& )
{
    if ( masterObserver_ && waveformObserver_ ) {
        if ( ch1 || ch2 ) {
            auto pair = std::make_pair( ( ch1 ? ch1->shared_from_this() : nullptr ), ( ch2 ? ch2->shared_from_this() : nullptr ) );
            auto pos = (*waveformObserver_) << pair;
            masterObserver_->dataChanged( waveformObserver_.get(), pos );
            return false; // no next method changed.
        }
    }
    return false; // no next method changed.
}
