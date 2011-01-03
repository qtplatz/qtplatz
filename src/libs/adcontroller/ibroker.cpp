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

#include "ibroker.h"
#include "iproxy.h"
#include "oproxy.h"
#include <ace/Reactor.h>
#include <ace/Thread_Manager.h>
#include <adinterface/receiverC.h>
#include <adinterface/eventlogC.h>
#include <acewrapper/mutex.hpp>
#include "ibrokermanager.h"
#include "message.h"
#include <acewrapper/timeval.h>
#include <acewrapper/messageblock.h>
#include <iostream>
#include <sstream>
#include <adinterface/eventlog_helper.h>
#include "marshal.hpp"
#include "constants.h"
#include <adportable/configuration.h>
#include <adportable/configloader.h>
#include "observer_i.h"
#include "manager_i.h"
#include <acewrapper/orbservant.h>

using namespace adcontroller;

namespace adcontroller {
	namespace internal {

        struct session_data {
            bool operator == ( const session_data& ) const;
            bool operator == ( const Receiver_ptr ) const;
            bool operator == ( const ControlServer::Session_ptr ) const;
            ControlServer::Session_var session_;
            Receiver_var receiver_;
            session_data() {};
            session_data( const session_data& t ) : session_(t.session_), receiver_(t.receiver_) {};
        };

	}
}


iBroker::~iBroker()
{
    // this will block until a message arrives.
    // By blocking, we know that the destruction will be
    // paused until the last thread is done with the message
    // block
    ACE_Message_Block * mblk = 0;
    this->getq( mblk );
    ACE_Message_Block::release( mblk );
}

iBroker::iBroker( size_t n_threads ) : barrier_(n_threads)
                                     , n_threads_(n_threads) 
									 , status_current_( ControlServer::eNothing )
									 , status_being_( ControlServer::eNothing )  
{
}

void
iBroker::reset_clock()
{
	struct invoke_reset_clock {
		void operator ()( iproxy_ptr& proxy ) {
			proxy->reset_clock();
		}
	};

	acewrapper::scoped_mutex_t<> lock( mutex_ );
	std::for_each( iproxies_.begin(), iproxies_.end(), invoke_reset_clock() );
}

bool
iBroker::open()
{
    if ( activate( THR_NEW_LWP, n_threads_ ) != - 1 )
        return true;
    return false;
}

void
iBroker::close()
{
    msg_queue()->deactivate();
    ACE_Task<ACE_MT_SYNCH>::close( 0 );
}

bool
iBroker::setConfiguration( const wchar_t * xml )
{
    // if already has configuration, then error return
    if ( ! config_.xml().empty() )
        return false;

	status_current_ = status_being_ = ControlServer::eNotConfigured;

    const wchar_t * query = L"//Configuration[@name='InstrumentConfiguration']";
    adportable::ConfigLoader::loadConfigXML( config_, xml, query );
    
    return true;
}


bool
iBroker::configComplete()
{
    using namespace adportable;

	SignalObserver::Observer_var masterObserver = getObserver();
	if ( CORBA::is_nil( masterObserver.in() ) ) {
		assert(0);
		throw std::exception( "iBroker::configComplete - can't get master observer servant" );
	}

    int objid = 0;
    for ( Configuration::vector_type::iterator it = config_.begin(); it != config_.end(); ++it ) {
        Configuration & item = *it;

		++objid;

		// initialize instrument proxy
		boost::shared_ptr<iProxy> pProxy( new iProxy( *this ) );
		if ( pProxy ) {
			pProxy->objId( objid );
			pProxy->setConfiguration( item );
			acewrapper::scoped_mutex_t<> lock( mutex_ );
			iproxies_.push_back( pProxy );
		}

		// initialize observer proxy
		Instrument::Session_var iSession = pProxy->getSession();        
		boost::shared_ptr<oProxy> poProxy( new oProxy( *this ) );
		if ( poProxy ) {
			poProxy->objId( objid );
			poProxy->setConfiguration( item );
			if ( poProxy->setInstrumentSession( iSession ) ) { // assign objid to source objects
				size_t n = poProxy->populateObservers( objid );
				objid += n;
			}
			acewrapper::scoped_mutex_t<> lock( mutex_ );
			oproxies_.push_back( poProxy );

			// add source into the Cache
			masterObserver->addSibling( poProxy->getObject() );
		}

    }
	status_current_ = status_being_ = ControlServer::eConfigured;  // relevant modules are able to access.
    return true;
}

bool
iBroker::initialize()
{
	struct invoke_initialize {
		void operator ()( iproxy_ptr& proxy ) {
			proxy->initialize();
		}
		void operator ()( oproxy_ptr& proxy ) {
			proxy->initialize();
		}
	};

	acewrapper::scoped_mutex_t<> lock( mutex_ );
	std::for_each( iproxies_.begin(), iproxies_.end(), invoke_initialize() );
	std::for_each( oproxies_.begin(), oproxies_.end(), invoke_initialize() );
	return true;
}

bool
iBroker::connect( ControlServer::Session_ptr session, Receiver_ptr receiver, const wchar_t * token )
{
	internal::session_data data;
    data.session_ = ControlServer::Session::_duplicate( session );
    data.receiver_ = Receiver::_duplicate( receiver );
    
    acewrapper::scoped_mutex_t<> lock( mutex_ );
    
    if ( std::find(session_set_.begin(), session_set_.end(), data ) != session_set_.end() )
        return false;
    
    session_set_.push_back( data );

	// fire connect
	struct invoke_connect {
        const wchar_t * token_;
		invoke_connect( const wchar_t * token ) : token_(token) {}
		void operator ()( iproxy_ptr& proxy ) {
			proxy->connect( token_ );
		}
		void operator ()( oproxy_ptr& proxy ) {
			proxy->connect( token_ );
		}
	};

	std::for_each( iproxies_.begin(), iproxies_.end(), invoke_connect(token) );
	std::for_each( oproxies_.begin(), oproxies_.end(), invoke_connect(token) );

    do {
        using namespace adinterface::EventLog;

        LogMessageHelper log( L"A pair of session %1%, Receiver %2% has success connected" );
        log % static_cast< void * >( session ) % static_cast<void *>(receiver);
        ACE_Message_Block * mb = marshal< ::EventLog::LogMessage >::put( log.get(), constants::MB_EVENTLOG );

        this->putq( mb );

    } while(0);

    return true;
}

bool
iBroker::disconnect( ControlServer::Session_ptr session, Receiver_ptr receiver )
{
	internal::session_data data;
    data.session_ = ControlServer::Session::_duplicate( session );
    data.receiver_ = Receiver::_duplicate( receiver );
    
    acewrapper::scoped_mutex_t<> lock( mutex_ );
    
	session_vector_type::iterator it = std::remove( session_set_.begin(), session_set_.end(), data );
    
    if ( it != session_set_.end() ) {
        session_set_.erase( it, session_set_.end() );
        return true;
    }
    return false;
}

::ControlServer::eStatus
iBroker::getStatusCurrent()
{
	return status_current_;
}

::ControlServer::eStatus
iBroker::getStatusBeging()
{
	return status_being_;
}

bool
iBroker::observer_update_data( unsigned long objid, long pos )
{
    ACE_Message_Block * mb = new ACE_Message_Block(128);
    unsigned long * ulong = reinterpret_cast<unsigned long *>(mb->wr_ptr());
    int n = 0;
    ulong[n++] = objid;
    ulong[n++] = pos;
    mb->wr_ptr( reinterpret_cast<char *>(&ulong[n]) );
	mb->msg_type( constants::MB_OBSERVER_UPDATE_DATA );
	putq( mb );
	return true;
}

bool
iBroker::observer_update_method( unsigned long objid, long pos )
{
    ACE_Message_Block * mb = new ACE_Message_Block(128);
    unsigned long * ulong = reinterpret_cast<unsigned long *>(mb->wr_ptr());
    int n = 0;
    ulong[n++] = objid;
    ulong[n++] = pos;
    mb->wr_ptr( reinterpret_cast<char *>(&ulong[n]) );
	mb->msg_type( constants::MB_OBSERVER_UPDATE_METHOD );
	putq( mb );
	return true;
}

bool
iBroker::observer_update_event( unsigned long objid, long pos, unsigned long event )
{
    ACE_Message_Block * mb = new ACE_Message_Block(128);
    unsigned long * ulong = reinterpret_cast<unsigned long *>(mb->wr_ptr());
    int n = 0;
    ulong[n++] = objid;
    ulong[n++] = pos;
    ulong[n++] = event;
    mb->wr_ptr( reinterpret_cast<char *>(&ulong[n]) );
	mb->msg_type( constants::MB_OBSERVER_UPDATE_EVENT );
	putq( mb );
	return true;
}


///////////////////////////////////////////////////////////////

bool
internal::session_data::operator == ( const session_data& t ) const
{
    return receiver_->_is_equivalent( t.receiver_.in() )
        && session_->_is_equivalent( t.session_.in() );
}

bool
internal::session_data::operator == ( const Receiver_ptr t ) const
{
    return receiver_->_is_equivalent( t );
}

bool
internal::session_data::operator == ( const ControlServer::Session_ptr t ) const
{
    return session_->_is_equivalent( t );
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
int
iBroker::handle_input( ACE_HANDLE )
{
    ACE_Message_Block *mb;
    ACE_Time_Value zero( ACE_Time_Value::zero );
    if ( this->getq(mb, &zero) == -1 ) {
        ACE_ERROR((LM_ERROR, "(%t) %p\n", "dequeue_head"));
    } else {
        ACE_Message_Block::release(mb);
    }
    return 0;
}

int
iBroker::svc()
{
    std::cout << "iBroker::svc() task started on thread :" << ACE_Thread::self() << std::endl;
    
    barrier_.wait();
    
    for ( ;; ) {
        ACE_Message_Block * mblk = 0;
        
        if ( this->getq( mblk ) == (-1) ) {
            if ( errno == ESHUTDOWN )
                ACE_ERROR_RETURN((LM_ERROR, "(%t) queue is deactivated\n"), 0);
            else
                ACE_ERROR_RETURN((LM_ERROR, "(%t) %p\n", "putq"), -1);
        }

        if ( mblk->msg_type() == ACE_Message_Block::MB_HANGUP ) {
            this->putq( mblk ); // forward the request to any peer threads
            break;
        }

        doit( mblk );
        ACE_Message_Block::release( mblk );

    }
    return 0;
}

void
iBroker::doit( ACE_Message_Block * mblk )
{
    using namespace adcontroller;

    if ( mblk->msg_type() >= ACE_Message_Block::MB_USER ) {
        dispatch( mblk, mblk->msg_type() );
    } else if ( mblk->msg_type() == ACE_Message_Block::MB_DATA ) {
        Message msg;
        ACE_InputCDR cdr( mblk ); 
        cdr >> msg;

        std::ostringstream o;
    
        o << "doit <" << ACE_Thread::self() << "> : src:" << msg.seqId_ << " dst:" << msg.dstId_ 
          << " cmd:" << msg.cmdId_ << " seq:" <<  msg.seqId_;

        ACE_Time_Value tv;
        if ( msg.cmdId_ == Notify_Timeout ) {
            cdr >> tv;
            o << " tv=" << acewrapper::to_string( tv );
            for ( session_vector_type::iterator it = session_begin(); it != session_end(); ++it )
				it->session_->echo( o.str().c_str() );
        }
    }
}

void
iBroker::dispatch( ACE_Message_Block * mblk, int disp )
{
    switch( disp ) {
    case constants::MB_EVENTLOG:
        handle_dispatch( marshal< EventLog::LogMessage >::get( mblk ) );
        break;
    case constants::MB_TIME_VALUE:
        handle_dispatch( marshal< ACE_Time_Value >::get( mblk ) );
        break;
    case constants::MB_CONNECT:
        break;
	case constants::MB_MESSAGE:
		do {
			ACE_InputCDR cdr( mblk );
			ACE_CDR::WChar * name = 0;
			ACE_CDR::ULong msgid, value;
            
            cdr.read_wstring( name );
            cdr.read_ulong( msgid );
            cdr.read_ulong( value );
			handle_dispatch( name, msgid, value );
		} while ( 0 );
		break;
	case constants::MB_OBSERVER_UPDATE_DATA:
		do {
			unsigned long * pUlong = reinterpret_cast<unsigned long *>( mblk->rd_ptr() );
			handle_observer_update_data( pUlong[0], pUlong[1] );
		} while(0);
		break;
	case constants::MB_OBSERVER_UPDATE_METHOD:
		do {
			unsigned long * pUlong = reinterpret_cast<unsigned long *>( mblk->rd_ptr() );
			handle_observer_update_method( pUlong[0], pUlong[1] );
		} while(0);
		break;
    default:
        break;
    };
}

/////////////////
void
iBroker::handle_dispatch( const std::wstring& name, unsigned long msgid, unsigned long value )
{
    ACE_UNUSED_ARG( name );
    acewrapper::scoped_mutex_t<> lock( mutex_ );    

	// TODO: apply barrier pattern and wait until all instrument has same state
/*
		    , HEARTBEAT // formerly, it was timer signal
		    , CONFIG_CHANGED
		    , STATE_CHANGED
		    , METHOD_RECEIVED
		    , METHOD_STARTED
		    , START_IN
		    , START_OUT
		    , INJECT_IN
		    , INJECT_OUT
		    , EVENT_IN
		    , EVENT_OUT
            , SETPTS_UPDATED
            , ACTUAL_UPDATED
*/
    // Following is just a quick debugging --> trigger spectrum display, should be removed
	// Right code is implement SignalObserver and UpdateData event is the right place to issue event.
	for ( session_vector_type::iterator it = session_begin(); it != session_end(); ++it )
		it->receiver_->message( Receiver::eINSTEVENT(msgid), value );
}

void
iBroker::handle_dispatch( const EventLog::LogMessage& msg )
{
    acewrapper::scoped_mutex_t<> lock( mutex_ );    
    for ( session_vector_type::iterator it = session_begin(); it != session_end(); ++it )
        it->receiver_->log( msg );
}

void
iBroker::handle_dispatch( const ACE_Time_Value& tv )
{
/*
    using namespace adinterface::EventLog;
    std::string tstr = acewrapper::to_string( tv );
    std::wstring wstr( tstr.size() + 1, L'\0' );
    std::copy( tstr.begin(), tstr.end(), wstr.begin() );

    LogMessageHelper log(tv);
    log.format( L"Time_Value: %1% handled in TASK %2%" ) % wstr % ACE_Thread::self();
    ::EventLog::LogMessage& msg = log.get();

    acewrapper::scoped_mutex_t<> lock( mutex_ );    
    for ( session_vector_type::iterator it = session_begin(); it != session_end(); ++it )
        it->receiver_->log( msg );
*/
}

void
iBroker::handle_observer_update_data( unsigned long id, long pos )
{
	pMasterObserver_->invoke_update_data( id, pos );
}

void
iBroker::handle_observer_update_method( unsigned long id, long pos )
{
	pMasterObserver_->invoke_method_changed( id, pos );
}

void
iBroker::handle_observer_update_event( unsigned long id, long pos, unsigned long event )
{
	pMasterObserver_->invoke_update_event( id, pos, event );
}


SignalObserver::Observer_ptr
iBroker::getObserver()
{
	PortableServer::POA_var poa = adcontroller::singleton::manager::instance()->poa();
	if ( ! pMasterObserver_ ) {
  		acewrapper::scoped_mutex_t<> lock( mutex_ );
		if ( ! pMasterObserver_ )
			pMasterObserver_.reset( new observer_i() );
	}
	CORBA::Object_ptr obj = poa->servant_to_reference( pMasterObserver_.get() );
    try {
        return SignalObserver::Observer::_narrow( obj );
    } catch ( CORBA::Exception& ) {
    }
    return 0;
}