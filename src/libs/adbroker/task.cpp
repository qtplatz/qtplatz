//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "task.h"
#include <acewrapper/mutex.hpp>
#include <acewrapper/mutex.hpp>
#include "message.h"
#include <sstream>
#include <acewrapper/timeval.h>
#include <iostream>

#pragma warning(disable:4996)
# include <ace/Reactor.h>
# include <ace/Thread_Manager.h>
# include <adinterface/receiverC.h>
#pragma warning(default:4996)

using namespace adbroker;

Task::~Task()
{
}

Task::Task( size_t n_threads ) : barrier_(n_threads)
                               , n_threads_(n_threads) 
{
}

int
Task::handle_timer_timeout( const ACE_Time_Value& tv, const void * )
{
    (void)tv;
	return 0;
}

bool
Task::open()
{
	if ( activate( THR_NEW_LWP, n_threads_ ) != - 1 )
		return true;
	return false;
}

void
Task::close()
{
    do {
        // this will block until a message arrives.
        // By blocking, we know that the destruction will be
        // paused until the last thread is done with the message
        // block
        ACE_Message_Block * mblk = new ACE_Message_Block( 0, ACE_Message_Block::MB_HANGUP );
        putq( mblk );
    } while (0);

    this->wait();
    this->msg_queue()->deactivate();
    ACE_Task<ACE_MT_SYNCH>::close( 0 );

    delete this;
}

bool
Task::connect( Broker::Session_ptr session, BrokerEventSink_ptr receiver )
{
	session_data data;
	data.session_ = Broker::Session::_duplicate( session );
    data.receiver_ = BrokerEventSink::_duplicate( receiver );

	acewrapper::scoped_mutex_t<> lock( mutex_ );

	if ( std::find(session_set_.begin(), session_set_.end(), data ) != session_set_.end() )
		return false;
  
	session_set_.push_back( data );

	return true;
}

bool
Task::disconnect( Broker::Session_ptr session, BrokerEventSink_ptr receiver )
{
	session_data data;
    data.session_ = Broker::Session::_duplicate( session );
    data.receiver_ = BrokerEventSink::_duplicate( receiver );

	acewrapper::scoped_mutex_t<> lock( mutex_ );

    vector_type::iterator it = std::remove( session_set_.begin(), session_set_.end(), data );

	if ( it != session_set_.end() ) {
		session_set_.erase( it, session_set_.end() );
		return true;
	}
	return false;

}

///////////////////////////////////////////////////////////////
bool
Task::session_data::operator == ( const session_data& t ) const
{
	return receiver_->_is_equivalent( t.receiver_.in() )
		&& session_->_is_equivalent( t.session_.in() );
}

bool
Task::session_data::operator == ( const BrokerEventSink_ptr t ) const
{
	return receiver_->_is_equivalent( t );
}

bool
Task::session_data::operator == ( const Broker::Session_ptr t ) const
{
	return session_->_is_equivalent( t );
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
int
Task::handle_input( ACE_HANDLE )
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
Task::svc()
{
    std::cerr << "Task::svc() task started on thread :" << ACE_Thread::self() << std::endl;

	barrier_.wait();

	for ( ;; ) {

		ACE_Message_Block * mblk = 0;

        if ( this->getq( mblk ) == (-1) ) {
			if ( errno == ESHUTDOWN )
                ACE_ERROR_RETURN((LM_ERROR, "(%t) adbroker::task queue is deactivated\n"), 0);
			else
				ACE_ERROR_RETURN((LM_ERROR, "(%t) %p\n", "putq"), -1);
		}

		if ( mblk->msg_type() == ACE_Message_Block::MB_HANGUP ) {
            std::cerr << "adbroker::task close on thread :" << ACE_Thread::self() << std::endl;
            this->putq( mblk ); // forward the request to any peer threads
			break;
		}
		doit( mblk );
		ACE_Message_Block::release( mblk );
	}
	return 0;
}

void
do_addSpectrum( SignalObserver::Observer_ptr observer, double x1, double x2 )
{
}

void
Task::doit( ACE_Message_Block * mblk )
{
    using namespace adbroker;

    TAO_InputCDR cdr( mblk );

    CORBA::WString_var msg;
    cdr >> msg;
    if ( std::wstring( msg ) == L"addSpectrum" ) {
        SignalObserver::Observer_ptr observer;
        double x1(0), x2(0);
        cdr >> observer;
        cdr >> x1;
        cdr >> x2;
        do_addSpectrum( observer, x1, x2 );
    }
}

