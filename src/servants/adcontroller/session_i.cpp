/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
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

#if defined _MSC_VER
# pragma warning (disable: 4996)
#endif
#include "session_i.hpp"
#include "adinterface/receiverC.h"
#include "adinterface/signalobserverC.h"
#if defined _MSC_VER
# pragma warning (default: 4996)
#endif

#include <iostream>
#include "ibrokermanager.hpp"
#include "ibroker.hpp"
#include <acewrapper/mutex.hpp>
#include <boost/tokenizer.hpp>

using namespace acewrapper;
using namespace adcontroller;

////////////////////////////////////////////

bool
session_i::receiver_data::operator == ( const receiver_data& t ) const
{
    return receiver_->_is_equivalent( t.receiver_.in() );
}

bool
session_i::receiver_data::operator == ( const Receiver_ptr t ) const
{
    return receiver_->_is_equivalent( t );
}

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

    scoped_mutex_t<> lock( singleton::iBrokerManager::instance()->mutex() );

    // check session_i local receiver, if already exist then error
    receiver_data data;
    data.receiver_ = Receiver::_duplicate( receiver );
    if ( std::find(receiver_set_.begin(), receiver_set_.end(), data) != receiver_set_.end() ) {
        throw ControlServer::Session::CannotAdd( L"receiver already exist" );
        return false;
    }

    // try connect to server
    iBroker * pBroker = singleton::iBrokerManager::instance()->get<iBroker>();
	if ( ! pBroker->connect( _this(), receiver, token ) ) {
        throw ControlServer::Session::CannotAdd( L"receiver already exist" );
        return false;
    }

    // now, it is safe to keep a copy for connection
    receiver_set_.push_back( data );
    
    return true;
}

CORBA::Boolean
session_i::disconnect( Receiver_ptr receiver )
{
    return internal_disconnect( receiver );
}

CORBA::Boolean
session_i::setConfiguration( const CORBA::WChar * xml )
{
    using namespace adcontroller::singleton;
    return iBrokerManager::instance()->get<iBroker>()->setConfiguration( xml );
}

CORBA::Boolean
session_i::configComplete()
{
    using namespace adcontroller::singleton;
    return iBrokerManager::instance()->get<iBroker>()->configComplete();
}

CORBA::Boolean
session_i::initialize()
{
    using namespace adcontroller::singleton;
	//iBrokerManager::instance()->initialize();
	//return iBrokerManager::instance()->get<iBroker>()->open();
	return iBrokerManager::instance()->get<iBroker>()->initialize();
}

CORBA::Boolean
session_i::shutdown()
{
    using namespace adcontroller::singleton;
    iBrokerManager::instance()->manager_terminate();
    ACE_Thread_Manager::instance()->wait();
    return true;
}

::ControlServer::eStatus
session_i::status()
{
    using namespace adcontroller::singleton;
	return iBrokerManager::instance()->get<iBroker>()->getStatusCurrent();
}

CORBA::Boolean
session_i::echo( const char * msg )
{
    // lock required
    for ( vector_type::iterator it = begin(); it != end(); ++it ) {
        it->receiver_->debug_print( 0, 0, msg );
    }
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
session_i::prepare_for_run( const ControlMethod::Method_ptr )
{
    return true;
}

CORBA::Boolean
session_i::start_run()
{
    return false;
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
    return false;
}

/////////////////////////////////////////////
void
session_i::register_failed( vector_type::iterator& it )
{
    receiver_failed_.push_back( *it );
}

void
session_i::commit_failed()
{
    if ( ! receiver_failed_.empty() ) {
        for ( vector_type::iterator 
                  it = receiver_failed_.begin(); it != receiver_failed_.end(); ++it ) {
            internal_disconnect( it->receiver_ );
        }
        receiver_failed_.clear();
    }
}

bool
session_i::internal_disconnect( Receiver_ptr receiver )
{
    scoped_mutex_t<> lock( singleton::iBrokerManager::instance()->mutex() );
    
    vector_type::iterator it = std::find(receiver_set_.begin(), receiver_set_.end(), receiver);
    
    if ( it != receiver_set_.end() ) {
        receiver_set_.erase( it );
        return true;
    }
    return false;
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
    iBroker * pBroker = singleton::iBrokerManager::instance()->get<iBroker>();
    return pBroker->getObserver();
}
