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
#include <ap240/digitizer.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adportable/utf.hpp>
#include <adportable/debug.hpp>
#include <memory>
#include <sstream>

using namespace ap240controller::Instrument;

Session::Session() : hasSession_( false )
{
}

Session::~Session()
{
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
Session::connect( adicontroller::Receiver * receiver, const std::string& token )
{
    return false;
}

bool
Session::disconnect( adicontroller::Receiver * receiver )
{
    return false; //malpix4::instance()->clientDisconnect( receiver );    
}
      
uint32_t
Session::get_status()
{
    return 0;
}

adicontroller::SignalObserver::Observer *
Session::getObserver()
{
    return 0;
}
      
bool
Session::initialize()
{
    return true;
}

bool
Session::shutdown()
{
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

std::shared_ptr< const adcontrols::ControlMethod::Method>
Session::getControlMethod()
{
    return 0; // adicontroller::ControlMethod::Method();
}

bool
Session::prepare_for_run( std::shared_ptr< const adcontrols::ControlMethod::Method > m )
{
    return false;
}
    
bool
Session::event_out( uint32_t event )
{
    return true;
}

bool
Session::start_run()
{
    return true;
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
    return true;
}

