//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "ibroker.h"
#include <acewrapper/mutex.hpp>
#include <ace/Reactor.h>
#include <ace/Thread_Manager.h>
#include <adinterface/receiverC.h>
#include <acewrapper/mutex.hpp>
#include "ibrokermanager.h"
#include "message.h"
#include <sstream>
#include <acewrapper/timeval.h>
#include <iostream>

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
{
}

void
iBroker::reset_clock()
{
}

int
iBroker::handle_timer_timeout( const ACE_Time_Value& tv, const void * )
{
    ACE_Message_Block * mb = new ACE_Message_Block( sizeof(tv) );
	* reinterpret_cast< ACE_Time_Value *>( mb->wr_ptr() ) = tv;
    this->putq( mb );
	return 0;
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
iBroker::connect( ControlServer::Session_ptr session, Receiver_ptr receiver )
{
	session_data data;
	data.session_ = ControlServer::Session::_duplicate( session );
    data.receiver_ = Receiver::_duplicate( receiver );

	acewrapper::scoped_mutex_t<> lock( mutex_ );

	if ( std::find(session_set_.begin(), session_set_.end(), data ) != session_set_.end() )
		return false;
  
	session_set_.push_back( data );

	return true;
}

bool
iBroker::disconnect( ControlServer::Session_ptr session, Receiver_ptr receiver )
{
	session_data data;
	data.session_ = ControlServer::Session::_duplicate( session );
    data.receiver_ = Receiver::_duplicate( receiver );

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
iBroker::session_data::operator == ( const session_data& t ) const
{
	return receiver_->_is_equivalent( t.receiver_.in() )
		&& session_->_is_equivalent( t.session_.in() );
}

bool
iBroker::session_data::operator == ( const Receiver_ptr t ) const
{
	return receiver_->_is_equivalent( t );
}

bool
iBroker::session_data::operator == ( const ControlServer::Session_ptr t ) const
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
    using namespace ns_adcontroller;

	std::ostringstream o;

    Message msg;
    ACE_InputCDR cdr( mblk ); 
    cdr >> msg;

	o << "doit <" << ACE_Thread::self() << "> : src:" << msg.seqId_ << " dst:" << msg.dstId_ 
		<< " cmd:" << msg.cmdId_ << " seq:" <<  msg.seqId_;

	if ( msg.cmdId_ == Notify_Timeout ) {
		ACE_Time_Value tv;
        cdr >> tv;
		o << " tv=" << acewrapper::to_string( tv );
	}
	for ( vector_type::iterator it = begin(); it != end(); ++it ) {
		it->session_->echo( o.str().c_str() );
	}

}