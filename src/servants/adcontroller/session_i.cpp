/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
**
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include "constants.hpp"
#include "taskmanager.hpp"
#include "task.hpp"
#include <acewrapper/mutex.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>

using namespace acewrapper;
using namespace adcontroller;

////////////////////////////////////////////

session_i::~session_i()
{
}

session_i::session_i()
{
    // singleton::Task::instance()->initialize();
}

CORBA::WChar *
session_i::software_revision()
{
    return const_cast<wchar_t *>( L"1.0" );
}

CORBA::Boolean
session_i::connect( Receiver_ptr receiver, const CORBA::WChar * token )
{
    ACE_UNUSED_ARG(token);

    if ( ! iTaskManager::instance()->task().connect( _this(), receiver, token ) ) {
        throw ControlServer::Session::CannotAdd( L"receiver already exist" );
        return false;
    }
    return true;
}

CORBA::Boolean
session_i::disconnect( Receiver_ptr receiver )
{
    return iTaskManager::instance()->task().disconnect( _this(), receiver );
}

CORBA::Boolean
session_i::setConfiguration( const CORBA::WChar * xml )
{
    return iTaskManager::instance()->get<iTask>()->setConfiguration( xml );
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
    iTaskManager::instance()->manager_terminate();
    ACE_Thread_Manager::instance()->wait();
    return true;
}

::ControlServer::eStatus
session_i::status()
{
    return iTaskManager::instance()->task().getStatusCurrent();
}

CORBA::Boolean
session_i::echo( const char * msg )
{
    ACE_OutputCDR cdr;
    using namespace adcontroller::constants;

    cdr << SESSION_COMMAND_ECHO;
    cdr << msg;
    ACE_Message_Block * mb = cdr.begin()->duplicate();
    mb->msg_type( MB_COMMAND );
    iTask::instance()->putq( mb );

    return true;
}

typedef boost::tokenizer< boost::char_separator<char> > tokenizer;

CORBA::Boolean
session_i::shell( const char * cmdline )
{
    boost::char_separator<char> separator("\t ", "");
    
    std::string cmdLine( cmdline );
    tokenizer commands(cmdLine, separator);
    
    tokenizer::const_iterator cmds = commands.begin();
    
    // todo
    
    return false;
}

//---------
CORBA::Boolean
session_i::prepare_for_run( const ControlMethod::Method& m )
{
    TAO_OutputCDR cdr;
    using namespace adcontroller::constants;

    cdr << SESSION_COMMAND_INITRUN;
    cdr << m;
    ACE_Message_Block * mb = cdr.begin()->duplicate();
    mb->msg_type( MB_COMMAND );
    iTask::instance()->putq( mb );
    return true;
}

CORBA::Boolean
session_i::start_run()
{
    ACE_OutputCDR cdr;
    using namespace adcontroller::constants;

    cdr << SESSION_COMMAND_STARTRUN;
    ACE_Message_Block * mb = cdr.begin()->duplicate();
    mb->msg_type( MB_COMMAND );
    iTask::instance()->putq( mb );
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
    ACE_OutputCDR cdr;
    using namespace adcontroller::constants;

    cdr << SESSION_COMMAND_STOPRUN;
    ACE_Message_Block * mb = cdr.begin()->duplicate();
    mb->msg_type( MB_COMMAND );
    iTask::instance()->putq( mb );
    return true;
}

bool
session_i::event_out( CORBA::ULong value )
{
    ACE_UNUSED_ARG( value );
    return false;
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
    return iTaskManager::instance()->get<iTask>()->getObserver();
}
