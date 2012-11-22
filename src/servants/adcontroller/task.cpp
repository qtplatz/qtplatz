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

#include "task.hpp"
#include "iproxy.hpp"
#include "oproxy.hpp"
#include "logging.hpp"
#include <ace/Reactor.h>
#include <ace/Thread_Manager.h>
#include <adinterface/receiverC.h>
#include <adinterface/eventlogC.h>
#include <adinterface/samplebrokerC.h>
#include <acewrapper/mutex.hpp>
#include "taskmanager.hpp"
#include "message.hpp"
#include <acewrapper/timeval.hpp>
#include <acewrapper/messageblock.hpp>
#include <iostream>
#include <sstream>
#include <adinterface/eventlog_helper.hpp>
#include "marshal.hpp"
#include "constants.hpp"
#include <adportable/configuration.hpp>
#include <adportable/configloader.hpp>
#include <adportable/debug.hpp>
#include <adportable/timer.hpp>
#include "observer_i.hpp"
#include "manager_i.hpp"
#include <acewrapper/orbservant.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <stdexcept>
#if defined _DEBUG
# include <iostream>
#endif

using namespace adcontroller;

iTask * iTask::instance_ = 0;

namespace adcontroller {
    namespace internal {
	
        struct receiver_data {
            bool operator == ( const receiver_data& ) const;
            bool operator == ( const Receiver_ptr ) const;
            bool operator == ( const ControlServer::Session_ptr ) const;
            ControlServer::Session_var session_;
            Receiver_var receiver_;
            std::wstring token_;
            size_t failed_;
            receiver_data() : failed_( 0 ) {}
            receiver_data( const receiver_data& t )
                : session_(t.session_), receiver_(t.receiver_), token_( t.token_ ), failed_( t.failed_ ) {
            }
        };
	
    }
}

iTask::~iTask()
{
    // this will block until a message arrives.
    // By blocking, we know that the destruction will be
    // paused until the last thread is done with the message
    // block
    ACE_Message_Block * mblk = 0;
    this->getq( mblk );
    ACE_Message_Block::release( mblk );
}

iTask::iTask( size_t n_threads ) : barrier_(n_threads)
                                 , n_threads_(n_threads) 
                                 , status_current_( ControlServer::eNothing )
                                 , status_being_( ControlServer::eNothing )  
{
    instance_ = this;
}

iTask *
iTask::instance()
{
    if ( ! instance_ )
        return iTaskManager::instance()->get<iTask>();
    return instance_;
}

void
iTask::reset_clock()
{
    using adcontroller::iTask;

    acewrapper::scoped_mutex_t<> lock( mutex_ );
    std::for_each( iproxies_.begin(), iproxies_.end(), boost::bind( &iProxy::reset_clock, _1 ) );
}

bool
iTask::open()
{
    if ( activate( THR_NEW_LWP, n_threads_ ) != - 1 )
        return true;
    return false;
}

void
iTask::close()
{
    msg_queue()->deactivate();
    ACE_Task<ACE_MT_SYNCH>::close( 0 );
}

bool
iTask::setConfiguration( const wchar_t * xml )
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
iTask::configComplete()
{
    return true;
}

bool
iTask::initialize_configuration()
{
    using namespace adportable;

    if ( status_current_ >= ControlServer::eConfigured )
        return true;

	adportable::timer x;
	Logging(L"iTask::initialize_configuration...", ::EventLog::pri_DEBUG );

    SignalObserver::Observer_var masterObserver = getObserver();
    if ( CORBA::is_nil( masterObserver.in() ) ) {
        assert(0);
        throw std::runtime_error( "iTask::initialize_configuration - can't get master observer servant" );
    }

    int objid = 0;
    for ( Configuration::vector_type::iterator it = config_.begin(); it != config_.end(); ++it ) {
        Configuration & item = *it;

        ++objid;
        // initialize instrument proxy
        boost::shared_ptr<iProxy> pProxy( new iProxy( *this ) );
        if ( pProxy ) {
			adportable::timer timer;
            pProxy->objId( objid );
            if ( ! pProxy->initialConfiguration( item ) ) {
                Logging(L"iTask::initialize_configuration -- instrument initialization failed for \"%1%\""
                        , ::EventLog::pri_WARNING ) % item.name();
				continue; //return false;
            }
            acewrapper::scoped_mutex_t<> lock( mutex_ );
            iproxies_.push_back( pProxy );
            Logging(L"iTask::initialize_configuration -- instrument \"%1%\" successfully initialized as objId %2% took %3% us"
                    , ::EventLog::pri_INFO )  % item.name() % objid % timer.elapsed();
        }

        // initialize observer proxy
        Instrument::Session_var iSession = pProxy->getSession();
        if ( ! CORBA::is_nil( iSession.in() ) ) {
            boost::shared_ptr<oProxy> poProxy( new oProxy( *this ) );
            if ( poProxy ) {
				adportable::timer timer;

                poProxy->objId( objid );
                poProxy->setConfiguration( item );
                if ( poProxy->setInstrumentSession( iSession ) ) { // assign objid to source objects
                    size_t n = poProxy->populateObservers( objid );
                    Logging(L"iTask::initialize_configuration -- \"%1%\" has %2% signal observers %3% us"
						, ::EventLog::pri_INFO ) % item.name() % n % timer.elapsed();
                    objid += n;
                }
                acewrapper::scoped_mutex_t<> lock( mutex_ );
                oproxies_.push_back( poProxy );

                // add source into the Cache (1st layer siblings)
                masterObserver->addSibling( poProxy->getObject() );
            }
        }
    }

    // fire connect
    using adcontroller::iProxy;
    using adcontroller::oProxy;
    std::for_each( iproxies_.begin(), iproxies_.end(), boost::bind( &iProxy::connect, _1, L"adcontroller.iTask" ) );
    std::for_each( oproxies_.begin(), oproxies_.end(), boost::bind( &oProxy::connect, _1, L"adcontroller.iTask" ) );

    status_current_ = status_being_ = ControlServer::eConfigured;  // relevant modules are able to access.
	Logging(L"iTask::initialize_configuration completed. %1% us", ::EventLog::pri_INFO ) % x.elapsed();
    return true;
}

bool
iTask::initialize()
{
    using adcontroller::iProxy;
    using adcontroller::oProxy;

    Logging(L"iTask::initialize...", ::EventLog::pri_INFO );
    if ( initialize_configuration() ) {
        acewrapper::scoped_mutex_t<> lock( mutex_ );
        std::for_each( iproxies_.begin(), iproxies_.end(), boost::bind( &iProxy::initialize, _1 ) );
        std::for_each( oproxies_.begin(), oproxies_.end(), boost::bind( &oProxy::initialize, _1 ) );
        return true;
    }
    return false;
}

bool
iTask::connect( ControlServer::Session_ptr session, Receiver_ptr receiver, const wchar_t * token )
{
    internal::receiver_data data;
    data.session_ = ControlServer::Session::_duplicate( session );
    data.receiver_ = Receiver::_duplicate( receiver );
    data.token_ = token;
    
    acewrapper::scoped_mutex_t<> lock( mutex_ );
    
    if ( std::find(receiver_set_.begin(), receiver_set_.end(), data ) != receiver_set_.end() )
        return false;

    receiver_set_.push_back( data );
    
    Logging( L"A pair of session %1%, Receiver %2% from \"%3%\" has success connected"
             , EventLog::pri_INFO ) % static_cast< void * >( session ) % static_cast<void *>( receiver ) % token;

    return true;
}

bool
iTask::disconnect( ControlServer::Session_ptr session, Receiver_ptr receiver )
{
    internal::receiver_data data;
    data.session_ = ControlServer::Session::_duplicate( session );
    data.receiver_ = Receiver::_duplicate( receiver );
    
    acewrapper::scoped_mutex_t<> lock( mutex_ );
    
    receiver_vector_type::iterator it = std::remove( receiver_set_.begin(), receiver_set_.end(), data );
    
    if ( it != receiver_set_.end() ) {
        receiver_set_.erase( it, receiver_set_.end() );
        return true;
    }
    return false;
}

::ControlServer::eStatus
iTask::getStatusCurrent()
{
    return status_current_;
}

::ControlServer::eStatus
iTask::getStatusBeging()
{
    return status_being_;
}

bool
iTask::observer_update_data( unsigned long parentId, unsigned long objid, long pos )
{
    ACE_Message_Block * mb = new ACE_Message_Block(128);
    unsigned long * ulong = reinterpret_cast<unsigned long *>(mb->wr_ptr());
    int n = 0;
    ulong[n++] = parentId;
    ulong[n++] = objid;
    ulong[n++] = pos;
    mb->wr_ptr( reinterpret_cast<char *>(&ulong[n]) );
    mb->msg_type( constants::MB_OBSERVER_UPDATE_DATA );
    putq( mb );
    return true;
}

bool
iTask::observer_update_method( unsigned long parentId, unsigned long objid, long pos )
{
    ACE_Message_Block * mb = new ACE_Message_Block(128);
    unsigned long * ulong = reinterpret_cast<unsigned long *>(mb->wr_ptr());
    int n = 0;
    ulong[n++] = parentId;
    ulong[n++] = objid;
    ulong[n++] = pos;
    mb->wr_ptr( reinterpret_cast<char *>(&ulong[n]) );
    mb->msg_type( constants::MB_OBSERVER_UPDATE_METHOD );
    putq( mb );
    return true;
}

bool
iTask::observer_update_event( unsigned long parentId, unsigned long objid, long pos, unsigned long events )
{
    ACE_Message_Block * mb = new ACE_Message_Block(128);
    unsigned long * ulong = reinterpret_cast<unsigned long *>(mb->wr_ptr());
    int n = 0;
    ulong[n++] = parentId;
    ulong[n++] = objid;
    ulong[n++] = pos;
    ulong[n++] = events;
    mb->wr_ptr( reinterpret_cast<char *>(&ulong[n]) );
    mb->msg_type( constants::MB_OBSERVER_UPDATE_EVENT );
    putq( mb );
    return true;
}


///////////////////////////////////////////////////////////////

bool
internal::receiver_data::operator == ( const receiver_data& t ) const
{
    return receiver_->_is_equivalent( t.receiver_.in() )
        && session_->_is_equivalent( t.session_.in() );
}

bool
internal::receiver_data::operator == ( const Receiver_ptr t ) const
{
    return receiver_->_is_equivalent( t );
}

bool
internal::receiver_data::operator == ( const ControlServer::Session_ptr t ) const
{
    return session_->_is_equivalent( t );
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
int
iTask::handle_input( ACE_HANDLE )
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
iTask::svc()
{
    std::cout << "iTask::svc() task started on thread :" << ACE_Thread::self() << std::endl;
    
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
        dispatch( mblk );
        ACE_Message_Block::release( mblk );
    }
    return 0;
}

void
iTask::dispatch( ACE_Message_Block * mblk )
{
    switch( mblk->msg_type() ) {
    case constants::MB_EVENTLOG:
        handle_dispatch( marshal< EventLog::LogMessage >::get( mblk ) );
        break;
    case constants::MB_COMMAND:
        handle_dispatch_command( mblk );
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
            handle_observer_update_data( pUlong[0], pUlong[1], pUlong[2] );
        } while(0);
        break;
    case constants::MB_OBSERVER_UPDATE_METHOD:
        do {
            unsigned long * pUlong = reinterpret_cast<unsigned long *>( mblk->rd_ptr() );
            handle_observer_update_method( pUlong[0], pUlong[1], pUlong[2] );
        } while(0);
        break;
    case constants::MB_OBSERVER_UPDATE_EVENT:
        do {
            unsigned long * pUlong = reinterpret_cast<unsigned long *>( mblk->rd_ptr() );
            handle_observer_update_events( pUlong[0], pUlong[1], pUlong[2], pUlong[3] );
        } while(0);
        break;
    default:
        break;
    };
}

/////////////////
void
iTask::handle_dispatch( const std::wstring& name, unsigned long msgid, unsigned long value )
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
    BOOST_FOREACH( internal::receiver_data& d, this->receiver_set_ ) {
        try {
            d.receiver_->message( Receiver::eINSTEVENT( msgid ), value );
        } catch ( CORBA::Exception& ex ) {
            d.failed_++;
            adportable::debug(__FILE__, __LINE__) << "exception: " << ex._name();
        }
    }
}

void
iTask::handle_dispatch( const EventLog::LogMessage& msg )
{
	// adportable::debug() << "iTask::handle_dispatch( EventLog: "  << msg.format.in() << " )";
	// Logging( L"adcontroller::iTask::handle_dispatch EventLog: " + std::wstring( msg.format.in() ), ::EventLog::pri_INFO );
    acewrapper::scoped_mutex_t<> lock( mutex_ );    

    BOOST_FOREACH( internal::receiver_data& d, receiver_set_ ) {
        try {
            d.receiver_->log( msg );
        } catch ( CORBA::Exception& ) {
            d.failed_++;
        }
    }
}

void
iTask::handle_dispatch( const ACE_Time_Value& )
{
}

void
iTask::handle_dispatch_command( ACE_Message_Block * mblk )
{
    TAO_InputCDR cdr( mblk );
    unsigned int cmd;
    cdr >> cmd;
    if ( cmd == constants::SESSION_COMMAND_ECHO ) {
        ACE_CString s;
        cdr >> s;
        using internal::receiver_data;
        BOOST_FOREACH( receiver_data& d, receiver_set_ ) {
            try {
                d.receiver_->debug_print( 0, 0, s.c_str() );
            } catch ( CORBA::Exception& ex ) {
                d.failed_++;
                adportable::debug(__FILE__, __LINE__) << "iTask::handle_dispatch_command 'echo' got an exception";
            }
        }
    } else if ( cmd == constants::SESSION_COMMAND_INITRUN ) {
        acewrapper::scoped_mutex_t<> lock( mutex_ );
        SampleBroker::SampleSequenceLine s;
        ControlMethod::Method m;
        std::for_each( iproxies_.begin(), iproxies_.end(), boost::bind( &adcontroller::iProxy::prepare_for_run, _1, s, m ) );
    } else if ( cmd == constants::SESSION_COMMAND_STARTRUN ) {
        acewrapper::scoped_mutex_t<> lock( mutex_ );
        std::for_each( iproxies_.begin(), iproxies_.end(), boost::bind( &adcontroller::iProxy::startRun, _1 ) );
    } else if ( cmd == constants::SESSION_COMMAND_STOPRUN )  {
        acewrapper::scoped_mutex_t<> lock( mutex_ );
        std::for_each( iproxies_.begin(), iproxies_.end(), boost::bind( &adcontroller::iProxy::stopRun, _1 ) );
    }
}

void
iTask::handle_observer_update_data( unsigned long parentId, unsigned long objId, long pos )
{
    pMasterObserver_->handle_data( parentId, objId, pos );
    pMasterObserver_->forward_observer_update_data( parentId, objId, pos );
}

void
iTask::handle_observer_update_method( unsigned long parentId, unsigned long objId, long pos )
{
    pMasterObserver_->forward_observer_update_method( parentId, objId, pos );
}

void
iTask::handle_observer_update_events( unsigned long parentId, unsigned long objId, long pos, unsigned long events )
{
    pMasterObserver_->forward_observer_update_events( parentId, objId, pos, events );
}


SignalObserver::Observer_ptr
iTask::getObserver()
{
    PortableServer::POA_var poa = adcontroller::manager_i::instance()->poa();
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
