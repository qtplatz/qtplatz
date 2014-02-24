/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
**
** Contact: info@ms-cheminfo.com
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

#include "session_i.hpp"
#include "adinterface/receiverC.h"
#include "adinterface/signalobserverC.h"
#include "task.hpp"
#include <boost/tokenizer.hpp>
#include <iostream>

using namespace adcontroller;

////////////////////////////////////////////

session_i::~session_i()
{
}

session_i::session_i()
{
}

CORBA::Char *
session_i::software_revision()
{
    return CORBA::string_dup("2.0");
}

CORBA::Boolean
session_i::connect( Receiver_ptr receiver, const CORBA::Char * token )
{
    ACE_UNUSED_ARG(token);

    if ( ! iTask::instance()->connect( _this(), receiver, token ) ) {
        throw ControlServer::Session::CannotAdd( L"receiver already exist" );
        return false;
    }
    return true;
}

CORBA::Boolean
session_i::disconnect( Receiver_ptr receiver )
{
    return iTask::instance()->disconnect( _this(), receiver );
}

CORBA::Boolean
session_i::setConfiguration( const char * xml )
{
    return iTask::instance()->setConfiguration( xml );
}

CORBA::Boolean
session_i::configComplete()
{
    return iTask::instance()->configComplete();
}

CORBA::Boolean
session_i::initialize()
{
    return iTask::instance()->initialize();
}

CORBA::Boolean
session_i::shutdown()
{
    iTask::instance()->close();
    return true;
}

::ControlServer::eStatus
session_i::status()
{
    return iTask::instance()->getStatusCurrent();
}

CORBA::Boolean
session_i::echo( const char * msg )
{
    iTask::instance()->io_service().post( std::bind(&iTask::handle_echo, iTask::instance(), std::string( msg ) ) );
    return true;
}

typedef boost::tokenizer< boost::char_separator<char> > tokenizer;

CORBA::Boolean
session_i::shell( const char * cmdline )
{
	(void)cmdline;
    return false;
}

//---------
CORBA::Boolean
session_i::prepare_for_run( const ControlMethod::Method& m )
{
    iTask::instance()->io_service().post( std::bind(&iTask::handle_prepare_for_run, iTask::instance(), m ) );
    return true;
}

CORBA::Boolean
session_i::start_run()
{
    iTask::instance()->io_service().post( std::bind(&iTask::handle_start_run, iTask::instance() ) );
    return true;
}

CORBA::Boolean
session_i::suspend_run()
{
    return false;
}

CORBA::Boolean
session_i::resume_run()
{
    return false;
}

CORBA::Boolean
session_i::stop_run()
{
    iTask::instance()->io_service().post( std::bind(&iTask::handle_stop_run, iTask::instance() ) );
    return true;
}

bool
session_i::event_out( CORBA::ULong value )
{
    iTask::instance()->io_service().post( std::bind(&iTask::handle_event_out, iTask::instance(), value ) );
    return true;
}

bool
session_i::push_back( SampleBroker::SampleSequence_ptr sequence )
{
    ACE_UNUSED_ARG( sequence );
    return false;
}

::SignalObserver::Observer *
session_i::getObserver (void)
{
    return iTask::instance()->getObserver();
}
