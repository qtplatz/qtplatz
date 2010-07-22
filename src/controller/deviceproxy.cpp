// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "deviceproxy.h"
#include "eventreceiver.h"
#include <acewrapper/eventhandler.h>
#include <acewrapper/dgramhandler.h>
#include <acewrapper/reactorthread.h>
#include <acewrapper/lifecycle_frame_serializer.h>
#include <ace/Message_Block.h>
#include <acewrapper/ace_string.h>

#include <boost/smart_ptr.hpp>
#include <acewrapper/messageblock.h>
#include <ace/Singleton.h>
#include <ace/Reactor.h>

using namespace acewrapper;
using namespace adportable::protocol;

DeviceProxy::DeviceProxy( const ACE_INET_Addr& remote ) : remote_addr_(remote)
                                                        , lifeCycle_( 0x100 ) 
{
}

void
DeviceProxy::mcast_update_device( const LifeCycleFrame& frame, const LifeCycleData& data )
{
	// MCAST handler 
    ACE_UNUSED_ARG(frame);
    using namespace acewrapper;

	if ( ! dgramHandler_ )
		initialize();

    LifeCycleCommand replyCmd;
    LifeCycleState newState;

	if ( lifeCycle_.current_state() == LCS_CLOSED || lifeCycle_.current_state() == LCS_LISTEN ) {

		lifeCycle_.dispatch_received_data( data, newState, replyCmd );  // CONN_SYN | CONN_SYN_ACK | DATA | DATA_ACK | CLOSE | CLOSE_ACK

		LifeCycleState next;
		if ( lifeCycle_.apply_command( CONN_SYN, next ) ) // reset internal sequence number, set own state to SYN_SENT
			lifeCycle_.current_state( next );             // update internal state enum

        LifeCycleData reqData;
        lifeCycle_.prepare_reply_data( CONN_SYN, reqData, 0 );

        boost::intrusive_ptr<ACE_Message_Block> mb( lifecycle_frame_serializer::pack( reqData ) );

		std::string msg = LifeCycleHelper::to_string( reqData );
		std::string addr = acewrapper::string( remote_addr_ );
		msg += " sent to ";
        msg += addr;

        dgramHandler_->send( mb->rd_ptr(), mb->length(), remote_addr_ );

		emit signal_dgram_to_device( remote_addr_string_, QString(local_addr_string_.c_str()), msg.c_str() );

    }
}

bool
DeviceProxy::initialize()
{
   dgramHandler_.reset( new acewrapper::EventHandler< acewrapper::DgramReceiver<QEventReceiver> >() );
   if ( dgramHandler_ ) {
      int port = 6000;
      while ( ! dgramHandler_->open(port++) && port < 6999 )
          ;
      if ( port >= 6999 )
          return false;
      assert( port < 6999 );
      ACE_Reactor * reactor = acewrapper::TheReactorThread::instance()->get_reactor();
      if ( reactor )
          reactor->register_handler( dgramHandler_.get(), ACE_Event_Handler::READ_MASK );

      local_addr_string_ = acewrapper::string( static_cast<const ACE_INET_Addr&>(*dgramHandler_) );
      remote_addr_string_ = acewrapper::string( remote_addr_ );

      do {
          connect( dgramHandler_.get()
              , SIGNAL( signal_dgram_input( ACE_Message_Block * ) )
              , this, SLOT( on_notify_dgram( ACE_Message_Block* ) ) );
      } while (0);
      
      emit signal_dgram_to_device( remote_addr_string_, QString(local_addr_string_.c_str()), "initialized" );
   }
   return true;
 }

void
DeviceProxy::on_notify_dgram( ACE_Message_Block * mb )
{
    acewrapper::scoped_mblock_ptr<> will_release_ptr( mb );

    LifeCycleData data;
    LifeCycleFrame frame;
	if ( lifecycle_frame_serializer::unpack( mb, frame, data ) ) {

		std::string o = LifeCycleHelper::to_string( data );
		emit signal_dgram_to_device( remote_addr_string_, QString(local_addr_string_.c_str()), o.c_str() );

		LifeCycleCommand replyCmd;
        LifeCycleState newState;
		if ( lifeCycle_.dispatch_received_data( data, newState, replyCmd ) )
			lifeCycle_.current_state( newState );

		if ( replyCmd != NOTHING ) {
			LifeCycleData reqData;
			lifeCycle_.prepare_reply_data( replyCmd, reqData, 0 );

			boost::intrusive_ptr<ACE_Message_Block> mb( lifecycle_frame_serializer::pack( reqData ) );
			dgramHandler_->send( mb->rd_ptr(), mb->length(), remote_addr_ );
		}

		if ( LifeCycleHelper::command( data ) == DATA ) {
			// dispatch data, to be added
		}

	}
}
