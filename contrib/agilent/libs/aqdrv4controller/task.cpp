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

#include "task.hpp"
#include "waveformobserver.hpp"
#include <adicontroller/masterobserver.hpp>
#include <acqrscontrols/acqiris_client.hpp>
#include <acqrscontrols/acqiris_method.hpp>
#include <acqrscontrols/acqiris_waveform.hpp>
#include <adicontroller/receiver.hpp>
#include <adportable/debug.hpp>
#include <iostream>

using namespace aqdrv4controller;

task::~task()
{
}

task::task() : work_( io_service_ )
             , strand_( io_service_ )
             , masterObserver_( std::make_shared< adicontroller::MasterObserver >( "master.aqdrv4.ms-cheminfo.com" ) )
             , waveformObserver_( std::make_shared< WaveformObserver >() )
{
    masterObserver_->addSibling( waveformObserver_.get() );
}

task *
task::instance()
{
    static task __instance;
    return &__instance;
}

bool
task::initialize()
{
    using namespace std::chrono_literals;
    
    static std::once_flag flag;
    
    std::call_once( flag, [&]() {
            unsigned nCores = 2; //std::thread::hardware_concurrency();
            while( nCores-- )
                threads_.emplace_back( std::thread( [&]{ io_service_.run(); } ) );
        } );

    return true;
}

bool
task::finalize()
{
    if ( client_ )
        client_->stop();
    
    io_service_.stop();
    
    for ( auto& t: threads_ )
        t.join();

    return true;
}

void
task::connect( const std::string& server, const std::string& port )
{
    using namespace acqrscontrols;
    
    static std::once_flag flag;

    std::call_once( flag, [&](){

            client_ = std::make_unique< aqdrv4::acqiris_client >( server, port );
            
            client_->connect( [&]( const aqdrv4::preamble * preamble, const char * data, size_t length ){

                    if ( preamble->clsid == aqdrv4::waveform::clsid() ) {
                
                        if ( auto p = aqdrv4::protocol_serializer::deserialize< aqdrv4::waveform >( *preamble, data ) )
                            push( p );
                
                    } else if ( preamble->clsid == aqdrv4::acqiris_method::clsid() ) {
                
                        if ( auto p = aqdrv4::protocol_serializer::deserialize< aqdrv4::acqiris_method >( *preamble, data ) )
                            push( p );
                
                    } else if ( preamble->clsid == aqdrv4::clsid_temperature ) {
                
                        aqdrv4::pod_reader reader( data, preamble->length );
                        // document::instance()->replyTemperature( reader.get< int32_t >() );
                
                    }
                });

            threads_.emplace_back( std::thread( std::bind( &aqdrv4::acqiris_client::run, client_.get() ) ) );
        });
    
    initialize();

    reply_message( adicontroller::Receiver::STATE_CHANGED, adicontroller::Instrument::eStandBy );
}    
    
void
task::prepare_for_run( std::shared_ptr< const acqrscontrols::ap240::method > m )
{
    if ( client_ ) {
        acqrscontrols::aqdrv4::acqiris_method aqrs4m;
        
        *aqrs4m.mutable_trig() = m->trig_;
        *aqrs4m.mutable_hor() = m->hor_;
        *aqrs4m.mutable_ext() = m->ext_;
        
        if ( m->channels_ & 0x01 )
            *aqrs4m.mutable_ch1() = m->ch1_;
        
        if ( m->channels_ & 0x02 )
            *aqrs4m.mutable_ch2() = m->ch2_;

        if ( auto data = acqrscontrols::aqdrv4::protocol_serializer::serialize( aqrs4m ) ) 
            client_->write( data );

        waveformObserver_->prepare_for_run( aqrs4m.methodNumber_, m );
    }
}

#if 0
void
task::prepare_for_run( std::shared_ptr< const acqrscontrols::aqdrv4::acqiris_method > m, acqrscontrols::aqdrv4::SubMethodType )
{
    if ( client_ ) {
        if ( auto data = acqrscontrols::aqdrv4::protocol_serializer::serialize( *m ) )
            client_->write( data );
        // no ap240::method --> no method number registered!
    }
}
#endif

void
task::event_out( uint32_t events )
{
    if ( client_ ) {
        auto data = std::make_shared< acqrscontrols::aqdrv4::acqiris_protocol >();
        
        data->preamble().clsid = acqrscontrols::aqdrv4::clsid_event_out;
        *data << events;

        client_->write( data );
    }
}

void
task::push( std::shared_ptr< acqrscontrols::aqdrv4::waveform > d )
{
    using namespace std::chrono_literals;
    
    if ( masterObserver_ && waveformObserver_ ) {
        auto pos = waveformObserver_->push_back( d );  // this is not right though...
        masterObserver_->dataChanged( waveformObserver_.get(), pos );
    }
}

void
task::push( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > m )
{
    ADDEBUG() << "##### TODO Got method (adapted) from device --> reply data to client";
    reply_message( adicontroller::Receiver::METHOD_RECEIVED, receivers_.size() );    
}

void
task::connect_client( std::shared_ptr< adicontroller::Receiver > receiver, const std::string& token )
{
    {
        std::lock_guard< std::mutex > lock( mutex_ );
        receivers_.emplace_back( receiver, token );
    }
    reply_message( adicontroller::Receiver::CLIENT_ATTACHED, receivers_.size() );
}

void
task::disconnect_client( std::shared_ptr< adicontroller::Receiver > receiver )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    auto it = std::remove_if( receivers_.begin(), receivers_.end(), [&]( receiver_pair_t& a ){ return a.first == receiver; } );
    receivers_.erase( it, receivers_.end() );
}

void
task::reply_message( int msg, int value )
{
    io_service_.post( [&](){
            std::lock_guard< std::mutex > lock( mutex_ );
            for ( auto& receiver: receivers_ )
                receiver.first->message( adicontroller::Receiver::eINSTEVENT( msg ), value );
        });
}

