//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "i8ttask.h"
#include "tofsession.h"
#include "marshal.hpp"
#include "constants.h"

#include <ace/Reactor.h>
#include <acewrapper/constants.h>
#include <acewrapper/mutex.hpp>
#include <acewrapper/timeval.h>
#include <acewrapper/orbservant.h>
#include <adinterface/receiverC.h>
#include <orbsvcs/CosNamingC.h>
#include <acewrapper/nameservice.h>
#include <acewrapper/reactorthread.h>
#include <acewrapper/ace_string.h>
#include <adportable/string.h>

#include <adportable/protocollifecycle.h>
#include <acewrapper/lifecycle_frame_serializer.h>

using namespace tofcontroller;

i8tTask::i8tTask( size_t n ) : n_threads_(n)
                             , barrier_( n )
{
}

i8tTask::~i8tTask(void)
{
    ACE_Reactor * reactor = this->reactor();
    this->reactor( 0 );
	delete reactor;
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
	if ( activate( THR_NEW_LWP, n_threads_ ) != - 1 ) {
		return true;
	}
	return false;
}

void
i8tTask::close()
{
    if ( reactor() )
		reactor()->end_reactor_event_loop();
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
i8tTask::handle_timeout( const ACE_Time_Value& tv, const void * )
{
    ACE_Message_Block * mb = new ACE_Message_Block( sizeof(tv) );
	* reinterpret_cast< ACE_Time_Value *>( mb->wr_ptr() ) = tv;
    this->putq( mb );
	return 0;
}

int
i8tTask::handle_input( ACE_HANDLE h )
{
    const size_t size = 2000;
    ACE_Message_Block * mb = new ACE_Message_Block( size );
    ACE_Message_Block * pfrom = new ACE_Message_Block( sizeof( ACE_INET_Addr ) );
	ACE_INET_Addr * addr = new ( pfrom->wr_ptr() ) ACE_INET_Addr();

    int res = 0;
	if ( mcast_handler_ && ( h == mcast_handler_->get_handle() ) ) {

		mb->msg_type( constants::MB_MCAST );
		res = mcast_handler_->recv( mb->wr_ptr(), size, *addr );

	} else if ( dgram_handler_ && ( h == dgram_handler_->get_handle() ) ) {

		mb->msg_type( constants::MB_DGRAM );
		res = dgram_handler_->recv( mb->wr_ptr(), size, *addr );

	}
	if ( res == (-1) ) {
		DWORD err = GetLastError();
		ACE_Message_Block::release( mb );
		ACE_Message_Block::release( pfrom );
		return 0;
	}
	mb->length( res );
	mb->cont( pfrom );

	putq( mb );

	return 0;
}

void
i8tTask::internal_initialize()
{
	acewrapper::ORBServant< tofcontroller::tofSession_i >
		* pServant = tofcontroller::singleton::tofSession_i::instance();
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
	internal_initialize_timer();
	internal_initialize_mcast();
	internal_initialize_dgram();
}

bool
i8tTask::internal_initialize_reactor()
{
	ACE_Reactor * reactor = this->reactor();
	if ( reactor == 0 && !reactor_thread_ ) {
		reactor_thread_.reset( new acewrapper::ReactorThread() );
		acewrapper::ReactorThread::spawn( reactor_thread_.get() );
        reactor = reactor_thread_->get_reactor();
		this->reactor( reactor );
	}
	return true;
}

bool
i8tTask::internal_initialize_timer()
{
	if ( ! reactor() )
		internal_initialize_reactor();
	reactor()->schedule_timer( this, 0, ACE_Time_Value(3), ACE_Time_Value(3) );
	return true;
}

bool
i8tTask::internal_initialize_mcast()
{
	if ( ! reactor() )
		internal_initialize_reactor();
	if ( ! mcast_handler_ ) {
		mcast_handler_.reset( new acewrapper::McastHandler() );
		if ( mcast_handler_->open() )
			reactor()->register_handler( mcast_handler_->get_handle(), this, ACE_Event_Handler::READ_MASK );
	}
	return true;
}

bool
i8tTask::internal_initialize_dgram()
{
	if ( ! reactor() )
		internal_initialize_reactor();

	if ( ! dgram_handler_ ) {
		dgram_handler_.reset( new acewrapper::DgramHandler() );
        int port = 6000;
        while ( ! dgram_handler_->open( port++ ) && port < 6999 )
			;
		if ( port >= 6999 )
			return false;
		reactor()->register_handler( dgram_handler_->get_handle(), this, ACE_Event_Handler::READ_MASK );
	}
	return true;
}

int
i8tTask::svc()
{
	internal_initialize();
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
	if ( mblk->msg_type() == ACE_Message_Block::MB_DATA ) {


	} else if ( mblk->msg_type() >= ACE_Message_Block::MB_USER ) {
		switch( mblk->msg_type() ) {
		case constants::MB_COMMAND:
			dispatch_command( mblk );
			break;
		case constants::MB_MCAST:
            dispatch_mcast( mblk );
			break;
		case constants::MB_DGRAM:
            dispatch_dgram( mblk );
			break;
        case constants::MB_DEBUG:
			dispatch_debug( mblk );
            break;
		};
	}
}

void
i8tTask::dispatch_command( ACE_Message_Block * mblk )
{
	ACE_UNUSED_ARG(mblk);
/*
	using namespace constants;
	// SESSION_COMMAND cmd = static_cast<SESSION_COMMAND>( marshal<long>::get(mblk) );
	switch( cmd ) {
    case SESSION_INITIALIZE:
		command_initialize();
		break;
	};
*/
}

void
i8tTask::dispatch_debug( ACE_Message_Block * mblk )
{
	ACE_InputCDR cdr( mblk );
	CORBA::WChar * text, *key;
	cdr.read_wstring( text );
	cdr.read_wstring( key );

	::EventLog::LogMessage log;
	log.tv.sec = 0;
	log.tv.usec = 0;
	log.srcId = key;
	log.format = text;
	for ( vector_type::iterator it = receiver_set_.begin(); it != receiver_set_.end(); ++it ) {
		it->receiver_->log( log );
	}
}

void
i8tTask::dispatch_mcast( ACE_Message_Block * mb )
{
	ACE_Message_Block * pfrom = mb->cont();
	ACE_INET_Addr& from_addr( *reinterpret_cast<ACE_INET_Addr *>( pfrom->rd_ptr() ) );

	std::wstring key = adportable::string::convert( acewrapper::string( from_addr ) );

	using namespace adportable::protocol;
	using namespace acewrapper;
	LifeCycleData data;
	LifeCycleFrame frame;
   
	if ( lifecycle_frame_serializer::unpack( mb, frame, data ) ) {
		try {
			LifeCycle_Hello& hello = boost::get< LifeCycle_Hello& >(data);
			ACE_INET_Addr addr;
			addr.string_to_addr( hello.ipaddr_.c_str() );
			if ( addr.get_ip_address() == 0 ) {
				addr = from_addr;
				addr.set_port_number( hello.portnumber_ );
			}
			std::wstring text = adportable::string::convert( LifeCycleHelper::to_string( data ) );
            ACE_OutputCDR cdr;
            cdr.write_wstring( text.c_str() );
            cdr.write_wstring( key.c_str() );
            ACE_Message_Block * mblk = cdr.begin()->duplicate();
			mblk->msg_type( constants::MB_DEBUG );
            putq( mblk );
			// multicast_update_device( addr, frame, data );
		} catch ( std::bad_cast& ) {
		}
	}
}

void
i8tTask::dispatch_dgram( ACE_Message_Block * mblk )
{
	ACE_Message_Block * pfrom = mblk->cont();
	if ( pfrom ) {
		const ACE_INET_Addr& addr = *(reinterpret_cast< const ACE_INET_Addr *>(pfrom->rd_ptr()));
		std::string addr_str = acewrapper::string( addr );
		std::wstring key( addr_str.size(), L'\0' );
		std::copy( addr_str.begin(), addr_str.end(), key.begin() );


	}
}


void
i8tTask::command_initialize()
{
}