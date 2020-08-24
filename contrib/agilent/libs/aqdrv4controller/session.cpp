/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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
#include "task.hpp"
#include "waveformobserver.hpp"
#include <adcontrols/controlmethod.hpp>
#include <adacquire/masterobserver.hpp>
#include <adacquire/receiver.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/utf.hpp>
#include <adportable/debug.hpp>
#include <adportable/semaphore.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <atomic>
#include <future>
#include <memory>
#include <sstream>

using namespace aqdrv4controller::Instrument;

Session *
Session::instance()
{
    static std::shared_ptr< Session > __instance;
    static std::once_flag flag;

    std::call_once( flag, [&] () { __instance = std::make_shared< Session >(); } );
    return __instance.get();
}

Session::Session()
{
}

Session::~Session()
{
}

std::string
Session::software_revision() const
{
    return "3.3";
}

bool
Session::setConfiguration( const std::string& xml )
{
    using boost::property_tree::ptree;

    std::istringstream ss( xml );
    ptree pt;
    boost::property_tree::read_xml( ss, pt );

    if ( pt.get< bool >( "Digitizer.RemoteAccess" ) ) {
        if ( boost::optional< std::string > server = pt.get_optional< std::string >( "Digitizer.RemoteHost" ) )
            remote_server_ = server.get();

        if ( boost::optional< std::string > port = pt.get_optional< std::string >( "Digitizer.Port" ) )
            remote_port_ = port.get();

        if ( remote_port_.empty() )
            remote_port_ = "8010";
    }

    return true;
}

bool
Session::configComplete()
{
    return true;
}

bool
Session::disconnect( adacquire::Receiver * receiver )
{
    auto self( receiver->shared_from_this() );

    task::instance()->disconnect_client( self );

    return true;
}

uint32_t
Session::get_status()
{
    return 0;
}

adacquire::SignalObserver::Observer *
Session::getObserver()
{
    return task::instance()->masterObserver();
}

bool
Session::connect( adacquire::Receiver * receiver, const std::string& token )
{
    auto ptr( receiver->shared_from_this() );

    task::instance()->connect_client( ptr, token );

    return true;
}

bool
Session::initialize()
{
    if ( !remote_server_.empty() ) {

        static std::once_flag flag;

        std::call_once( flag, [&](){
                task::instance()->connect( remote_server_, remote_port_ );
            });
    }
    return true;
}

bool
Session::shutdown()
{
    return task::instance()->finalize();
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
    return 0;
}

bool
Session::prepare_for_run( std::shared_ptr< const adcontrols::ControlMethod::Method > m )
{
    if ( m ) {
        auto it = m->find( m->begin(), m->end(), acqrscontrols::ap240::method::clsid() ); // "ap240" );
        if ( it != m->end() ) {
            auto method = std::make_shared< acqrscontrols::ap240::method >();
            if ( it->get<>( *it, *method ) ) {

                task::instance()->prepare_for_run( method );

                return true;
            }
        }
    }
    return false;
}

bool
Session::time_event_trigger( std::shared_ptr< const adcontrols::ControlMethod::TimedEvents > tt
                             , adcontrols::ControlMethod::const_time_event_iterator begin
                             , adcontrols::ControlMethod::const_time_event_iterator end )
{
    ADDEBUG() << "TODO -- time_event_triger";
    return true;
}

bool
Session::event_out( uint32_t event )
{
    // if ( event == adacquire::Instrument::instEventInjectOut )
    task::instance()->event_out( event ); // trigger_inject();
    return true;
}

bool
Session::start_run()
{
    return true; //impl_->digitizer_->peripheral_run();
}

bool
Session::suspend_run()
{
    return true; // digitizer has no timed event.
}

bool
Session::resume_run()
{
    return true;  // digitizer has no timed event.
}

bool
Session::stop_run()
{
    return true; // impl_->digitizer_->peripheral_stop();
}

bool
Session::recording( bool enable )
{
    // if ( auto digi = impl_->digitizer_ ) {
    //     if ( enable )
    //         digi->peripheral_prepare_for_run();
    //     else
    //         digi->peripheral_stop();
    // }
    return true;
}

bool
Session::isRecording() const
{
    return true;
}
