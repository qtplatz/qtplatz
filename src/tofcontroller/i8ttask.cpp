//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "i8ttask.h"
#include <ace/Reactor.h>
#include <acewrapper/constants.h>
#include <acewrapper/mutex.hpp>
#include <acewrapper/timeval.h>
#include <acewrapper/orbservant.h>
#include <adinterface/receiverC.h>
#include <orbsvcs/CosNamingC.h>
#include "i8tmanager.h"
#include <acewrapper/nameservice.h>

using namespace tofcontroller;

i8tTask::i8tTask( size_t n ) : n_threads_(n)
                             , barrier_( n )
{
}

i8tTask::~i8tTask(void)
{
}

int
i8tTask::handle_timer_timeout( const ACE_Time_Value& tv, const void * )
{
    ACE_Message_Block * mb = new ACE_Message_Block( sizeof(tv) );
	* reinterpret_cast< ACE_Time_Value *>( mb->wr_ptr() ) = tv;
    this->putq( mb );
	return 0;
}

bool
i8tTask::setConfiguration( const wchar_t * xml )
{
	if ( ! configXML_.empty() )
		return false;
	configXML_ = xml;
	return true;
}

bool
i8tTask::open()
{
	if ( activate( THR_NEW_LWP, n_threads_ ) != - 1 )
		return true;
	return false;
}

void
i8tTask::close()
{
	msg_queue()->deactivate();
	ACE_Task<ACE_MT_SYNCH>::close( 0 );
}

bool
i8tTask::connect( Receiver_ptr receiver )
{
	receiver_data data;
    data.receiver_ = Receiver::_duplicate( receiver );

	acewrapper::scoped_mutex_t<> lock( mutex_ );

	if ( std::find(receiver_set_.begin(), receiver_set_.end(), data ) != receiver_set_.end() )
		return false;
  
	receiver_set_.push_back( data );

	return true;
}

bool
i8tTask::disconnect( Receiver_ptr receiver )
{
	receiver_data data;
    data.receiver_ = Receiver::_duplicate( receiver );

	acewrapper::scoped_mutex_t<> lock( mutex_ );

	vector_type::iterator it = std::remove( receiver_set_.begin(), receiver_set_.end(), data );

	if ( it != receiver_set_.end() ) {
		receiver_set_.erase( it, receiver_set_.end() );
		return true;
	}
	return false;

}

///////////////////////////////////////////////////////////////
bool
i8tTask::receiver_data::operator == ( const receiver_data& t ) const
{
	return receiver_->_is_equivalent( t.receiver_.in() );
}

bool
i8tTask::receiver_data::operator == ( const Receiver_ptr t ) const
{
	return receiver_->_is_equivalent( t );
}

////////////////////////////////////////////////////////////////

int
i8tTask::handle_input( ACE_HANDLE )
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

void
i8tTask::initialize()
{
	acewrapper::ORBServant< tofcontroller::i8tManager_i >
		* pServant = tofcontroller::singleton::i8tManager_i::instance();
	CORBA::ORB_var orb = pServant->orb();

	CORBA::Object_var obj;

	CosNaming::Name name = acewrapper::constants::adbroker::manager::name();
	CosNaming::NamingContext_var nc = acewrapper::NS::resolve_init( orb );
	obj = acewrapper::NS::resolve_name( nc, name );

	Broker::Manager_var manager = Broker::Manager::_narrow( obj.in() );

	if ( ! CORBA::is_nil( manager.in() ) ) {
		logger_ = manager->getLogger();

		if ( ! CORBA::is_nil( logger_.in() ) ) {

			Broker::LogMessage msg;
			msg.text = L"tofcontroller initialized";
			logger_->log( msg );
		}
	}
}

int
i8tTask::svc()
{
    initialize();
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
i8tTask::doit( ACE_Message_Block * mblk )
{
/*
    using namespace adbroker;

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
*/
}