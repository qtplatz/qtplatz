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
#include <adinterface/automaton.hpp>
#include <qtwrapper/settings.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/serialization/collection_size_type.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
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
        impl() : settings_( new QSettings( QSettings::IniFormat, QSettings::UserScope
                                           , QLatin1String( "eventtool" )
                                           , QLatin1String( "eventtool" ) ) )
               , automaton_( this )
               , instStatus_( adinterface::instrument::eOff ) {

            // don't call automaton_.start() in ctor -- it will call back handle_state() inside the singleton creation.
        }
            
        std::shared_ptr< QSettings > settings_;
        adinterface::fsm::controller automaton_;
        adinterface::instrument::eInstStatus instStatus_;

        // finite automaton handler
        void handle_state( bool entering, adinterface::instrument::eInstStatus ) override;
        void action_on( const adinterface::fsm::onoff& ) override;
        void action_prepare_for_run( const adinterface::fsm::prepare& ) override;
        void action_start_run( const adinterface::fsm::run& ) override;
        void action_stop_run( const adinterface::fsm::stop& ) override;
        void action_off( const adinterface::fsm::onoff& ) override;
        void action_diagnostic( const adinterface::fsm::onoff& ) override;
        void action_error_detected( const adinterface::fsm::error_detected& ) override;
    };
}


using namespace eventtool;

std::atomic<document *> document::instance_( 0 );
std::mutex document::mutex_;

document::document( QObject * parent ) : QObject( parent )
                                       , impl_( new impl )

{
}

document *
document::instance()
{
    document * tmp = instance_.load( std::memory_order_relaxed );
    std::atomic_thread_fence( std::memory_order_acquire );
    if ( tmp == nullptr ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        tmp = instance_.load( std::memory_order_relaxed );
        if ( tmp == nullptr ) {
            tmp = new document();
            std::atomic_thread_fence( std::memory_order_release );
            instance_.store( tmp, std::memory_order_relaxed );
        }
    }
    return tmp;
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

void
document::inject_event_out()
{
    QLibrary lib( "eventbroker" );

    if ( !lib.isLoaded() ) {
        if ( lib.load() ) {
            std::cout << lib.fileName().toStdString() << "\tloaded." << std::endl;
        }
    }
    if ( auto event_out = reinterpret_cast<void( *)(uint32_t)>(lib.resolve( "eventbroker_out" )) ) {
        event_out( 1 );
    }
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
