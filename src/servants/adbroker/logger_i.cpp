// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "logger_i.hpp"
#include <acewrapper/mutex.hpp>
#include <acewrapper/timeval.hpp>
#include <algorithm>
#include <assert.h>
#include <sstream>
# include <adinterface/loghandlerC.h>
# include <ace/OS_NS_time.h>
#include <iomanip>

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

    static const long long hlimit = (4000 - 1970) * 365LL * 1440LL * 60LL;

    if ( (m.tv_sec < 0 || m.tv_sec > hlimit ) || m.tv_sec == 0 ) {
        long usec;
	time_t tv_sec;
        acewrapper::gettimeofday( tv_sec, usec );
	m.tv_sec = tv_sec;
        m.tv_usec = usec;
    }

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
    time_t tv_sec = msg.tv_sec;
    char * sp = ACE_OS::ctime_r( &tv_sec, tbuf, sizeof(tbuf) );
    while ( *sp && *sp != '\n' )
        o << *sp++;
    o << L" " << std::fixed << std::setw(7) << std::setfill(L'0') << std::setprecision(3) << double( msg.tv_usec ) / 1000.0 << "\t: ";
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
