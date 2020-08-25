/**************************************************************************
** Copyright (C) 2013-2020 MS-Cheminformatics LLC
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
#include "singleton.hpp"
#include "digitizer.hpp"
#include "waveformobserver.hpp"
#include <aqmd3controls/method.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adacquire/masterobserver.hpp>
#include <adacquire/receiver.hpp>
#include <adlog/logger.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/utf.hpp>
#include <adportable/debug.hpp>
#include <adportable/semaphore.hpp>
#include <boost/asio.hpp>
#include <boost/exception/all.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <atomic>
#include <future>
#include <memory>
#include <sstream>
#include <string>

namespace aqmd3 {
    std::shared_ptr< session > session::instance_ = 0;
}

using namespace aqmd3;

session *
session::instance()
{
    static std::once_flag flag;
    std::call_once( flag, [&]{ instance_ = std::make_shared< session >(); } );
    return instance_.get();
}

session::session()
{
}

session::~session()
{
    instance_.reset();
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

    ADDEBUG() << "\t### setConfiguration(" << json << ")";

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
    auto cli( receiver->shared_from_this() );

    ADDEBUG() << "\n\n\n\t------------- " << __FUNCTION__ << " ---------------- " << token;

    singleton::instance()->initialize();

    if ( cli ) {
        singleton::instance()->connect( cli, token );
        return true;
    }
    return false;
}

bool
session::disconnect( adacquire::Receiver * receiver )
{
    auto self( receiver->shared_from_this() );
    return singleton::instance()->disconnect( self );
}

uint32_t
session::get_status()
{
    return 0;
}

adacquire::SignalObserver::Observer *
session::getObserver()
{
    //ADDEBUG() << "\t------------- " << __FUNCTION__ << " ----------------";
    return singleton::instance()->getObserver();
}

bool
session::initialize()
{
    //ADDEBUG() << "\n\n\t------------- sesson::initialize -> peripheral_initialize() ----------------";
    return singleton::instance()->digitizer().peripheral_initialize();
}

bool
session::shutdown()
{
    ADDEBUG() << "\t------------- " << __FUNCTION__ << " ----------------";
    singleton::instance()->digitizer().peripheral_terminate();
    singleton::instance()->finalize();
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
session::prepare_for_run( std::shared_ptr< const adcontrols::ControlMethod::Method > m )
{
    // ADDEBUG() << "\t=====================================================";
    // ADDEBUG() << "\t=============== " << __FUNCTION__ << " ===============";
    if ( m ) {
        auto it = m->find( m->begin(), m->end(), aqmd3controls::method::clsid() );
        if ( it != m->end() ) {
            aqmd3controls::method method;
            if ( it->get<>( *it, method ) )
                return singleton::instance()->digitizer().peripheral_prepare_for_run( method );
        } else {
            ADDEBUG() << "### " << __FUNCTION__ << " no aqmd3controls::method found.";
        }
    }
    return false;
}

bool
session::prepare_for_run( const std::string& json, arg_type atype )
{
    //ADDEBUG() << "\t------------- " << __FUNCTION__ << " ---------------- ==> TODO";
    assert(0);
    if ( atype != arg_json )
        return false;
    return true;
}

bool
session::event_out( uint32_t event )
{
    ADDEBUG() << "\t##### session::event_out( " << event << " )";
    return singleton::instance()->digitizer().peripheral_trigger_inject();
    return true;
}

bool
session::start_run()
{
    ADDEBUG() << "\t------------- " << __FUNCTION__ << " ----------------";
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
    //ADDEBUG() << "\t------------- " << __FUNCTION__ << " ----------------";
    return singleton::instance()->digitizer().peripheral_stop();
}

bool
session::time_event_trigger( std::shared_ptr< const adcontrols::ControlMethod::TimedEvents > tt
                             , adcontrols::ControlMethod::const_time_event_iterator begin
                             , adcontrols::ControlMethod::const_time_event_iterator end )
{
#ifndef NDEBUG
    ADDEBUG() << "TODO -- time_event_triger";
#endif
    return true;
}


bool
session::dark_run( size_t waitCount )
{
    return true; // impl_->digitizer_->peripheral_dark( waitCount );
}
