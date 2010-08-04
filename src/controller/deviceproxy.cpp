// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "deviceproxy.h"
#include "eventreceiver.h"
#include <sstream>
#include <acewrapper/eventhandler.h>
#include <acewrapper/dgramhandler.h>
#include <acewrapper/reactorthread.h>
#include <acewrapper/lifecycle_frame_serializer.h>
#include <ace/Message_Block.h>
#include <acewrapper/ace_string.h>
#include <acewrapper/outputcdr.h>

#include <boost/smart_ptr.hpp>
#include <acewrapper/messageblock.h>
#include <acewrapper/inputcdr.h>
#include <ace/Singleton.h>
#include <ace/Reactor.h>

#include "controllerC.h"
#include "tao/Utils/ORB_Manager.h"
#include "../device_emulator/reactor_thread.h"

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

	if ( lifeCycle_.machine_state() == LCS_CLOSED || lifeCycle_.machine_state() == LCS_LISTEN ) {

		// lifeCycle_.valid_heartbeat( ACE_High_Res_Timer::gettimeofday_hr() );

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
      ACE_Reactor * reactor = singleton::theReactorThread::instance()->get_reactor();
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

    TAO_InputCDR tao_input( mb );
    InputCDR input( tao_input );
	if ( lifecycle_frame_serializer::unpack( input, frame, data ) ) {

		std::string o = LifeCycleHelper::to_string( data );
		emit signal_dgram_to_device( remote_addr_string_, QString(local_addr_string_.c_str()), o.c_str() );

		LifeCycleCommand replyCmd;
        LifeCycleState newState;
		if ( lifeCycle_.dispatch_received_data( data, newState, replyCmd ) ) // state may change
			lifeCycle_.current_state( newState );

		if ( replyCmd != NOTHING ) {
			LifeCycleData reqData;
            if ( lifeCycle_.prepare_reply_data( replyCmd, reqData, 0 ) ) {
                lifeCycle_.apply_command( replyCmd, newState ); // state may change

                boost::intrusive_ptr<ACE_Message_Block> mb( lifecycle_frame_serializer::pack( reqData ) );
                dgramHandler_->send( mb->rd_ptr(), mb->length(), remote_addr_ );
            }
		}

		if ( LifeCycleHelper::command( data ) == DATA ) {
            unsigned long clsid;
            input >> clsid;
            if ( clsid == GlobalConstants::ClassID_InstEvent ) {
                A_Dummy_MSMethod::InstEvent e;
                tao_input >> e;

                std::ostringstream o;
                o << "event#" << e.eventId_ << ", value=" << e.eventValue_;
                emit signal_debug( remote_addr_string_, QString( o.str().c_str() ) );
             }
		}

	}
}

void
DeviceProxy::notify_timeout( const ACE_Time_Value& tv )
{
    Q_UNUSED(tv);
	if ( lifeCycle_.machine_state() == LCS_ESTABLISHED ) {

		LifeCycleData reqData;
        if ( lifeCycle_.prepare_data( reqData ) ) {

            TAO_OutputCDR tao_cdr;
            do {
                OutputCDR cdr( static_cast<ACE_OutputCDR&>(tao_cdr) );
                lifecycle_frame_serializer::pack( cdr, reqData );
            } while(0);

            static int count = 1;        

            A_Dummy_MSMethod::IonSourceMethod m;
            A_Dummy_MSMethod::ESIMethod esi;

            esi.nebulizing1_flow_ = 1.0 + double(count++) / 100;
            esi.nebulizing2_flow_ = 2.0;
            esi.needle1_voltage_ = 1000.0 + double(count) / 10;
            esi.needle2_voltage_ = 2000.0;
            m.esi_( esi );
            tao_cdr << GlobalConstants::ClassID_IonSourceMethod;
            tao_cdr << m;

            dgramHandler_->send( tao_cdr.begin()->rd_ptr(), tao_cdr.length(), remote_addr_ );
        }
    }
}
