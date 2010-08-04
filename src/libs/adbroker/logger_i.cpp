//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "logger_i.h"
#include <acewrapper/mutex.hpp>
#include <acewrapper/timeval.h>
#include <algorithm>
#include <assert.h>
#include <sstream>
#pragma warning (disable: 4996)
# include <adinterface/loghandlerC.h>
# include <ace/OS.h>
#pragma warning (default: 4996)

using namespace broker;
using namespace acewrapper;

////////////////////////////////////////////

bool
logger_i::handler_data::operator == ( const handler_data& t ) const
{
    return handler_->_is_equivalent( t.handler_.in() );
}

bool
logger_i::handler_data::operator == ( const LogHandler_ptr t ) const
{
    return handler_->_is_equivalent( t );
}

////////////////////////////////////////////

struct fire_update {
    unsigned long logId_;
    fire_update( unsigned long logId ) : logId_(logId) {}
    void operator()( logger_i::handler_data& h ) {
        h.handler_->notify_update( logId_ );
    }
};

/////////////////////////////////////////////


logger_i::logger_i( PortableServer::POA_ptr poa ) : poa_( PortableServer::POA::_duplicate(poa) )
                                                  , logId_(0)
{
}

logger_i::~logger_i(void)
{
}

void
logger_i::log( const Broker::LogMessage& msg )
{
    scoped_mutex_t<> lock( mutex_ );

    log_.push_back( msg );

    Broker::LogMessage& m = log_.back();
    m.logId = logId_++;

    long usec;
    acewrapper::gettimeofday( m.tv_sec, usec );
    m.tv_usec = usec;

    if ( log_.size() > 3000 )
        log_.erase( log_.begin() + 2000, log_.end() );

    std::for_each( handler_set_.begin(), handler_set_.end(), fire_update( m.logId ) );
}

bool
logger_i::findLog( CORBA::ULong logId, Broker::LogMessage& msg )
{
    scoped_mutex_t<> lock( mutex_ );

    if ( log_.empty() )
        return false;

    unsigned long front = log_.front().logId;
    unsigned long back = log_.back().logId;

    if ( logId < front || back < logId )
        return false;
    msg = log_[ logId - front ];
    assert( msg.logId == logId );
    return true;
}

bool
logger_i::nextLog( Broker::LogMessage& msg )
{
    return findLog( msg.logId, msg );
}

CORBA::WChar *
logger_i::to_string( const Broker::LogMessage& msg )
{
    std::wostringstream o;

    ACE_TCHAR tbuf[128];
    char * sp = ACE_OS::ctime_r( &msg.tv_sec, tbuf, sizeof(tbuf) );
    while ( *sp && *sp != '\n' )
        o << *sp++;
    o << L" " << double( msg.tv_usec ) / 1000 << "\t: ";
    o << msg.text.in();

    CORBA::WString_var s = CORBA::wstring_dup( o.str().c_str() );
    return s._retn();
}

bool
logger_i::register_handler( LogHandler_ptr handler )
{
    scoped_mutex_t<> lock( mutex_ );

    handler_data data;
    data.handler_ = LogHandler::_duplicate( handler );
      
    if ( std::find(handler_set_.begin(), handler_set_.end(), data) != handler_set_.end() ) {
        throw Broker::Logger::AlreadyExist( L"handler already exist" );
    } else {
        handler_set_.push_back( data );
        return true;
    }
    return false;
 }

bool
logger_i::unregister_handler( LogHandler_ptr handler )
{
    return internal_disconnect( handler );
}

bool
logger_i::internal_disconnect( LogHandler_ptr handler )
{
    scoped_mutex_t<> lock( mutex_ );

    vector_type::iterator it = std::remove( handler_set_.begin(), handler_set_.end(), handler );
    if ( it != handler_set_.end() ) {
        handler_set_.erase( it );
		return true;
	}
	return false;
}
