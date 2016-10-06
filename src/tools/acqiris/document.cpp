/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "document.hpp"
#include "digitizer.hpp"
#include "tcp_server.hpp"
#include "tcp_task.hpp"
#include <acqrscontrols/acqiris_client.hpp>
#include <acqrscontrols/acqiris_waveform.hpp>
#include <acqrscontrols/acqiris_method.hpp>
#include <acqrscontrols/acqiris_protocol.hpp>
#include <QSettings>
#include <adportable/debug.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/filesystem/path.hpp>
#include <chrono>
#include <fstream>
#include <iostream>

static std::once_flag flag;

document *
document::instance()
{
    static document __instance;
    return &__instance;
}

document::document( QObject * parent ) : QObject( parent )
                                       , method_( std::make_shared< acqrscontrols::aqdrv4::acqiris_method >() )
                                       , settings_( std::make_unique< QSettings >( QSettings::IniFormat
                                                                                   , QSettings::UserScope
                                                                                   , QLatin1String( "acqiris" )
                                                                                   , QLatin1String( "acqiris" )
                                                        ) )
                                       , temperature_( 0 )
{
}

document::~document()
{
}

bool
document::initialSetup()
{
    boost::filesystem::path path ( settings_->fileName().toStdString() );

    auto name = path.parent_path() / "acqiris_method.xml";

    if ( auto m = load( name.string() ) ) {

        set_acqiris_method( m );

    } else {

        m = std::make_shared< acqrscontrols::aqdrv4::acqiris_method >();

        auto trig = m->mutable_trig();
        auto hor = m->mutable_hor();
        auto ch1 = m->mutable_ch1();
        auto ext = m->mutable_ext();
        set_acqiris_method( m );

    }
    return true;
}

bool
document::finalClose()
{
    signal_final_close_();

    if ( ! tcp_threads_.empty() ) {
        if ( server_ )
            server_->stop();
        if ( client_ )
            client_->stop();
        for ( auto& t: tcp_threads_ )
            t.join();
    }

    boost::filesystem::path path ( settings_->fileName().toStdString() );
    auto name = path.parent_path() / "acqiris_method.xml";

    settings_->setValue( "DefaultMethod", QString::fromStdString( name.string() ) );
    
    save( name.string(), method_ );
    
    return true;
}

QSettings *
document::settings()
{
    return settings_.get();
}

void
document::push( std::shared_ptr< acqrscontrols::aqdrv4::waveform > d )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    
    que_.emplace_back( d ); // push should be called in strand so that no race should be exist

    if ( server_ )
        server_->post( d );
    
    if ( que_.size() >= 4096 ) {
        
        using namespace std::chrono_literals;
        static auto tp = std::chrono::steady_clock::now();
        if ( std::chrono::steady_clock::now() - tp > 10s ) {

            tp = std::chrono::steady_clock::now();
            double rate = ( que_.back()->timeStamp() - que_.front()->timeStamp() ) / ( que_.size() - 1 );
            ADDEBUG() << "average trig. interval: " << rate / std::nano::den << "s";
        }

        que_.erase( que_.begin(), que_.begin() + ( que_.size() - 2048 ) );
    }
}

std::shared_ptr< acqrscontrols::aqdrv4::waveform >
document::recentWaveform()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    
    if ( !que_.empty() )
        return que_.back();
    return nullptr;
}

std::shared_ptr< const acqrscontrols::aqdrv4::acqiris_method >
document::acqiris_method()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    return method_;
}

void
document::set_acqiris_method( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > p )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    method_ = p;
}

std::shared_ptr< const acqrscontrols::aqdrv4::acqiris_method >
document::adapted_acqiris_method()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    return adapted_method_;
}

void
document::acqiris_method_adapted( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > p )
{
    {
        std::lock_guard< std::mutex > lock( mutex_ );
        adapted_method_ = p;
    }
    emit on_acqiris_method_adapted();
}

void
document::set_server( std::unique_ptr< acqiris::server::tcp_server >&& server )
{
    server_ = std::move( server );
    std::call_once( flag, [&](){
            tcp_threads_.emplace_back( std::thread( [&]{ server_->run(); } ) );
        });
}

void
document::close_client()
{
    if ( client_ ) {
        client_->stop();
    }
}

void
document::set_client( std::unique_ptr< acqrscontrols::aqdrv4::acqiris_client >&& client )
{
    using namespace acqrscontrols;
    
    client_ = std::move( client );

    acqiris::client::tcp_task::instance()->initialize();

    std::call_once( flag, [&](){
            tcp_threads_.emplace_back( std::thread( [&]{ client_->run(); } ) );
        });
    
    client_->connect( [&]( const aqdrv4::preamble * preamble, const char * data, size_t length ){
            if ( preamble->clsid == aqdrv4::waveform::clsid() ) {
                
                if ( auto p = aqdrv4::protocol_serializer::deserialize< aqdrv4::waveform >( *preamble, data ) )
                    acqiris::client::tcp_task::instance()->push( p );
                
            } else if ( preamble->clsid == aqdrv4::acqiris_method::clsid() ) {
                
                if ( auto p = aqdrv4::protocol_serializer::deserialize< aqdrv4::acqiris_method >( *preamble, data ) )
                    acqiris::client::tcp_task::instance()->push( p );

            } else if ( preamble->clsid == acqrscontrols::aqdrv4::clsid_event_out ) {                

                ADDEBUG() << "## Error: Got event out from server.";
                
            } else if ( preamble->clsid == acqrscontrols::aqdrv4::clsid_temperature ) {
                
                aqdrv4::pod_reader reader( data, preamble->length );
                replyTemperature( reader.get< int32_t >() );
                
            }
        });
}

bool
document::save( const std::string& file, std::shared_ptr< const acqrscontrols::aqdrv4::acqiris_method > p )
{
    std::wofstream of( file );
    try {
        boost::archive::xml_woarchive ar( of );
        ar & boost::serialization::make_nvp("aqdrv4", *p );
    } catch ( ... ) {
        return false;
    }
    return true;
}

std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method >
document::load( const std::string& file )
{
    auto p = std::make_shared< acqrscontrols::aqdrv4::acqiris_method >();
    std::wifstream of( file );
    try {
        boost::archive::xml_wiarchive ar( of );
        ar & boost::serialization::make_nvp("aqdrv4", *p );
        return p;
    } catch ( ... ) {
        return nullptr;
    }
}

boost::signals2::connection
document::connect_prepare( const prepare_for_run_t::slot_type & subscriber )
{
    return signal_prepare_for_run_.connect( subscriber );
}

boost::signals2::connection
document::connect_event_out( const event_out_t::slot_type & subscriber )
{
    return signal_event_out_.connect( subscriber );
}

boost::signals2::connection
document::connect_finalize( const final_close_t::slot_type & subscriber )
{
    return signal_final_close_.connect( subscriber );
}

void
document::handleValueChanged( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > m, acqrscontrols::aqdrv4::SubMethodType subType )
{
    ADDEBUG() << "handleValueChanged";
    set_acqiris_method( m );
    signal_prepare_for_run_( m, subType );
}

void
document::handleEventOut( uint32_t event )
{
    ADDEBUG() << "handleEventOut(" << event << ")";
    signal_event_out_( event );
}

void
document::replyTemperature( int temp )
{
    if ( server_ ) {

        auto data = std::make_shared< acqrscontrols::aqdrv4::acqiris_protocol >();
        
        data->preamble().clsid = acqrscontrols::aqdrv4::clsid_temperature;
        *data << int32_t( temp );
        
        server_->post( data );
        
    } else {
        ADDEBUG() << "Temperature: " << temp;
    }
    
    if ( temp >= 58 )
        std::cout << "WARNING: Temperature " << temp << " too high" << std::endl;
        
    temperature_ = temp;
}
