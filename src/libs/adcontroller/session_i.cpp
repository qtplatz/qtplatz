//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma warning (disable: 4996)
#include "session_i.h"
#include "adinterface/receiverC.h"
#pragma warning (default: 4996)

#include <iostream>
#include "ibrokermanager.h"
#include "ibroker.h"
#include <acewrapper/mutex.hpp>
#include "ibroker.h"
#include <boost/tokenizer.hpp>

using namespace acewrapper;

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
}

CORBA::WChar *
session_i::software_revision()
{
   return const_cast<wchar_t *>( L"1.0" );
}

CORBA::Boolean
session_i::connect( Receiver_ptr receiver, const CORBA::WChar * token )
{
    scoped_mutex_t<> lock( iBrokerManager::instance()->mutex() );

    ACE_UNUSED_ARG(token);

    receiver_data data;
    data.receiver_ = Receiver::_duplicate( receiver );
      
    if ( std::find(receiver_set_.begin(), receiver_set_.end(), data) != receiver_set_.end() ) {
        throw ControlServer::Session::CannotAdd( L"receiver already exist" );
        return false;
    }
      
    receiver_set_.push_back( data );
      
    if ( receiver_set_.size() == 1 ) 
        iBrokerManager::instance()->get<iBroker>()->reset_clock();

    echo( "client connected" );
      
    return true;
}

CORBA::Boolean
session_i::disconnect( Receiver_ptr receiver )
{
    return internal_disconnect( receiver );
}

CORBA::Boolean
session_i::initialize()
{
    return iBrokerManager::instance()->get<iBroker>()->initialize();
}

CORBA::Boolean
session_i::shutdown()
{
    return false;
}

CORBA::ULong
session_i::get_status()
{
    return false;
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
	std::cerr << "Session_i::disconnect." << std::endl;

    scoped_mutex_t<> lock( iBrokerManager::instance()->mutex() );

	vector_type::iterator it = std::find(receiver_set_.begin(), receiver_set_.end(), receiver);
	if ( it != receiver_set_.end() ) {

#if defined DEBUG
		std::cerr << "--------------- Session_i::disconnect target found. size="
				  << receiver_set_.size() << std::endl;
#endif
		receiver_set_.erase( it );

#if defined DEBUG
		std::cerr << "--------------- Session_i::disconnect target erased. size="
				  << receiver_set_.size() << std::endl;
#endif

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

SignalObserver::Observer_ptr
session_i::getObserver()
{
    return 0;
}