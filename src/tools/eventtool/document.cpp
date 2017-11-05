/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <compiler/disable_4251.h>
#if defined _MSC_VER
#pragma warning(disable:4800 4503)
#endif
#include "document.hpp"
#include "eventtoolconstants.hpp"
#include "../eventbroker/eventbroker.h"
#include <acewrapper/udpeventreceiver.hpp>
#include <acewrapper/ifconfig.hpp>
#include <adinterface/automaton.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/debug.hpp>
#include <qtwrapper/settings.hpp>
#include <boost/asio.hpp>
#include <boost/exception/all.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/serialization/collection_size_type.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <QCoreApplication>
#include <QSettings>
#include <QMessageBox>
#include <QLibrary>
#include <cstdint>
#include <iostream>

namespace eventtool {

    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "eventtool";
        }
    };

    class document::impl : public adinterface::fsm::handler {
    public:
        ~impl() {
            io_service_.stop();
            for ( auto& t : threads_ )
                t.join();
        }

        impl() : settings_( new QSettings( QSettings::IniFormat, QSettings::UserScope
                                           , QLatin1String( "eventtool" ), QLatin1String( "eventtool" ) ) )
               , automaton_( this )
               , instStatus_( adinterface::instrument::eOff )
               , work_(io_service_) {
            // don't call automaton_.start() in ctor -- it will call back handle_state() inside the singleton creation.
        }

        std::shared_ptr< QSettings > settings_;
        adinterface::fsm::controller automaton_;
        adinterface::instrument::eInstStatus instStatus_;
        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
        std::vector< std::thread > threads_;
        std::unique_ptr< acewrapper::udpEventReceiver > udpReceiver_;
            
        // finite automaton handler
        void handle_state( bool entering, adinterface::instrument::eInstStatus ) override;
        void action_on( const adinterface::fsm::onoff& ) override;
        void action_prepare_for_run( const adinterface::fsm::prepare& ) override;
        void action_start_run( const adinterface::fsm::run& ) override;
        void action_stop_run( const adinterface::fsm::stop& ) override;
        void action_off( const adinterface::fsm::onoff& ) override;
        void action_diagnostic( const adinterface::fsm::onoff& ) override;
        void action_error_detected( const adinterface::fsm::error_detected& ) override;
        //
        void event_received( const char * data, size_t length, const boost::asio::ip::udp::endpoint& ep ) {
            std::string x( data, length );
            std::cout << x << "\tfrom " << ep << std::endl;
        }

        void monitor_enable( short port ) {
            udpReceiver_.reset( new acewrapper::udpEventReceiver( io_service_, port ) );
            udpReceiver_->register_handler( [this] ( const char * data, size_t length, const boost::asio::ip::udp::endpoint& ep ) { event_received( data, length, ep ); } );
            if ( threads_.empty() )
                threads_.push_back( adportable::asio::thread( [=]{ io_service_.run(); } ) );
        }
    };
}


using namespace eventtool;

std::atomic<document *> document::instance_( 0 );
std::mutex document::mutex_;

document::~document()
{
    delete impl_;
}

document::document( QObject * parent ) : QObject( parent )
                                       , impl_( new impl )

{
}

document *
document::instance()
{
    static document __instance;
    return &__instance;
}

void
document::initialSetup()
{
    boost::filesystem::path dir = user_preference::path( impl_->settings_.get() );
    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "eventtool::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
        }
    }
}

void
document::finalClose()
{
}

#if defined _DEBUG
const char * libs[] = {
    "eventbrokerd", "eventbroker", "libeventbrokerd", "libeventbroker"
};
#else
const char * libs[] = {
    "eventbroker", "eventbrokerd", "libeventbroker", "libeventbrokerd"
};
#endif

void
document::inject_event_out()
{
    static bool load_successed = false;

    auto path = QCoreApplication::applicationDirPath();
    std::cout << path.toStdString() << std::endl;
    QStringList list;
    for ( auto name: libs ) {
        list << name;
        list << QString("../lib/qtplatz/%1").arg( name );
    }
    
    for ( auto name: list ) {
        QLibrary lib( name );
        if ( lib.load() ) {
            if ( !load_successed ) {
                std::cout << lib.fileName().toStdString() << "\tloaded." << std::endl;
                load_successed = true;
            }
            if ( auto event_out = reinterpret_cast<bool( *)( uint32_t )>( lib.resolve( "eventbroker_out" ) ) ) {

                auto callback = [] ( const char * dllfunc, uint32_t rcode, double duration, const char * msg ) {
                    std::cout << boost::format( "%s: %s in %.3f milliseconds." ) % dllfunc % msg % (duration * 1000) << std::endl;
                };

                if ( auto register_handler = reinterpret_cast<bool(*)(event_handler)>( lib.resolve( "eventbroker_regiser_handler" ) ) )
                    register_handler( callback );

                event_out( 1 );

                if ( auto unregister_handler = reinterpret_cast<bool(*)(event_handler)>( lib.resolve( "eventbroker_unregiser_handler" ) ) )
                    unregister_handler( callback );

                return;
            } else {
                std::cout << lib.fileName().toStdString() << " can't resolve entry point for 'eventbroker_out'" << std::endl;
            }
        }
    }
    std::cout << "eventbroker.dll not found. Inject event can not be issued." << std::endl;
}

void
document::inject_bind( const std::string& host, const std::string& port )
{
    for ( auto name: libs ) {
        QLibrary lib( name );
        if ( lib.load() ) {
            if ( auto bind = reinterpret_cast<bool(*)(const char * host, const char * port)>(lib.resolve( "eventbroker_bind" )) ) {
                bind( host.c_str(), port.c_str() );
                std::cout << lib.fileName().toStdString() << "\tloaded, bind to " << host << ":" << port << std::endl;
                return;
            }            
        }
    }
    std::cout << "eventbroker.dll not found." << std::endl;    
}

bool
document::monitor_port( short port )
{
    try {
        impl_->monitor_enable( port );
        return true;
    }
    catch ( boost::exception& ex ) {
        ADDEBUG() << boost::diagnostic_information( ex );
        return false;
    }
}

void
document::monitor_disable()
{
    impl_->udpReceiver_.reset();
}

//////////////////////

void
document::impl::action_on( const adinterface::fsm::onoff& )
{
    std::cout << "action_on" << std::endl;
}

void
document::impl::action_prepare_for_run( const adinterface::fsm::prepare& )
{
    std::cout << "action_prepare_for_run" << std::endl;
}

void
document::impl::action_start_run( const adinterface::fsm::run& )
{
    std::cout << "action_start_run" << std::endl;
}

void
document::impl::action_stop_run( const adinterface::fsm::stop& )
{
    std::cout << "action_stop_run" << std::endl;
}

void
document::impl::action_off( const adinterface::fsm::onoff& )
{
    std::cout << "action_off" << std::endl;
}

void
document::impl::action_diagnostic( const adinterface::fsm::onoff& )
{
    std::cout << "action_diagnostic" << std::endl;
}

void
document::impl::action_error_detected( const adinterface::fsm::error_detected& )
{
    std::cout << "action_error_detected" << std::endl;
}

void
document::impl::handle_state( bool entering, adinterface::instrument::eInstStatus stat )
{
    if ( entering ) {
        instStatus_ = stat;
        emit document::instance()->instStateChanged( stat );
    }
}

void
document::initialized()
{
    acewrapper::ifconfig::ifvec addrs;
    if ( acewrapper::ifconfig::if_addrs( addrs ) ) {
        for ( auto& addr : addrs ) {
            std::cout << "interface: " << addr.first << "\t" << addr.second << std::endl;
        }
    }
}
