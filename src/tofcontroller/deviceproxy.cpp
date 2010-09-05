// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "deviceproxy.h"
#include "TOFTask.h"

#include <sstream>

#include <acewrapper/dgramhandler.h>

#include <acewrapper/lifecycle_frame_serializer.h>
#include <ace/Message_Block.h>
#include <acewrapper/ace_string.h>
#include <acewrapper/outputcdr.h>
#include <acewrapper/messageblock.h>
#include <acewrapper/inputcdr.h>
#include <acewrapper/ace_string.h>
#include <adportable/string.h>
#include "tofcontrollerC.h"

using namespace acewrapper;
//using namespace adportable::protocol;

using namespace tofcontroller;

DeviceProxy::DeviceProxy( const ACE_INET_Addr& remote
                        , TOFTask * task )
						: remote_addr_( remote )
						, lifeCycle_( 0x100 ) 
						, pTask_( task )
{
	name_ = adportable::string::convert( acewrapper::string( remote_addr_ ) );
}

ACE_HANDLE
DeviceProxy::get_handle() const
{
	if ( dgram_handler_ )
		return dgram_handler_->get_handle();
	return 0;
}

int
DeviceProxy::handle_input( ACE_HANDLE )
{
    const size_t size = 1024 * sizeof(long) * 256; // maximum packet

	ACE_Message_Block * mb = new ACE_Message_Block( size );
    ACE_Message_Block * pfrom = new ACE_Message_Block( sizeof( ACE_INET_Addr) );
	ACE_INET_Addr * pFromAddr = new (pfrom->wr_ptr()) ACE_INET_Addr();

	int res = dgram_handler_->recv( mb->wr_ptr(), size, *pFromAddr );
    if (res == (-1)) {
       perror("handle_input dgram.recv");
       ACE_Message_Block::release( mb );
       ACE_Message_Block::release( pfrom );
       return 0;
    }
    mb->length( res );
    mb->cont( pfrom );
    handle_lifecycle_dgram( mb );

	return 0;
}

int
DeviceProxy::handle_timeout( const ACE_Time_Value& current_time, const void * act )
{
    ACE_UNUSED_ARG( current_time );
    ACE_UNUSED_ARG( act );

	if ( lifeCycle_.machine_state() == adportable::protocol::LCS_ESTABLISHED ) {

		adportable::protocol::LifeCycleData reqData;
		if ( lifeCycle_.prepare_data( reqData ) ) {

            TAO_OutputCDR tao_cdr;
            do {
				OutputCDR cdr( static_cast<ACE_OutputCDR&>(tao_cdr) );
                lifecycle_frame_serializer::pack( cdr, reqData );
            } while(0);

            static int count = 1;        

			TOFInstrument::AnalyzerDeviceData data;
            // todo 
			// initialize data
     
			tao_cdr << TOFConstants::ClassID_AnalyzerDeviceData;
            tao_cdr << data;

			dgram_handler_->send( tao_cdr.begin()->rd_ptr(), tao_cdr.length(), remote_addr_ );
        }
    }

	return 0;
}

// static
DeviceProxy *
DeviceProxy::check_hello_and_create( ACE_Message_Block * mb
									, const ACE_INET_Addr& from_addr
									, TOFTask * task )
{
	adportable::protocol::LifeCycleData data;
	adportable::protocol::LifeCycleFrame frame;

	if ( acewrapper::lifecycle_frame_serializer::unpack( mb, frame, data ) ) {
		try {
			using namespace adportable::protocol;

			LifeCycle_Hello& hello = boost::get< LifeCycle_Hello& >(data);
			ACE_INET_Addr addr;
			addr.string_to_addr( hello.ipaddr_.c_str() );
			if ( addr.get_ip_address() == 0 ) {
				addr = from_addr;
				addr.set_port_number( hello.portnumber_ );
				DeviceProxy * p = new DeviceProxy( addr, task );
                p->initialize_dgram();
                p->handle_lifecycle_mcast( frame, data );
#if defined _DEBUG
				// std::wstring key = adportable::string::convert( acewrapper::string( from_addr ) );
				std::wstring text = adportable::string::convert( LifeCycleHelper::to_string( data ) );
                task->dispatch_debug( text, p->name() );
#endif
				return p; // error should be handled by caller
			}
		} catch ( std::bad_cast& ) {
		}
	}
    return 0;
}

bool
DeviceProxy::sendto( const ACE_Message_Block * mb )
{
	if ( ! dgram_handler_ )
		return false;
	size_t res = dgram_handler_->send( mb->rd_ptr(), mb->length(), remote_addr_ );
	if ( res != (-1) && ( res == mb->length() ) ) {
        // todo
        // wait ack
		return true;
	}
	return false;
}

bool
DeviceProxy::initialize_dgram()
{
	if ( ! dgram_handler_ ) {
		dgram_handler_.reset( new acewrapper::DgramHandler() );
        
        int port = 6000;
        while ( ! dgram_handler_->open( port++ ) && port < 6999 )
			;
		if ( port >= 6999 )
			return false;
	}
	return true;
}


void
DeviceProxy::handle_lifecycle_mcast( const adportable::protocol::LifeCycleFrame& frame
									, const adportable::protocol::LifeCycleData& data )
{
	// MCAST handler 
    ACE_UNUSED_ARG(frame);
    using namespace acewrapper;

	adportable::protocol::LifeCycleCommand replyCmd;
	adportable::protocol::LifeCycleState newState;

	using namespace adportable::protocol;

	if ( lifeCycle_.machine_state() == LCS_CLOSED || lifeCycle_.machine_state() == LCS_LISTEN ) {

		// lifeCycle_.valid_heartbeat( ACE_High_Res_Timer::gettimeofday_hr() );

		lifeCycle_.dispatch_received_data( data, newState, replyCmd );  // CONN_SYN | CONN_SYN_ACK | DATA | DATA_ACK | CLOSE | CLOSE_ACK

		LifeCycleState next;
		if ( lifeCycle_.apply_command( CONN_SYN, next ) ) // reset internal sequence number, set own state to SYN_SENT
			lifeCycle_.current_state( next );             // update internal state enum

        LifeCycleData reqData;
        lifeCycle_.prepare_reply_data( CONN_SYN, reqData, 0 );

        boost::intrusive_ptr<ACE_Message_Block> mb( lifecycle_frame_serializer::pack( reqData ) );

		dgram_handler_->send( mb->rd_ptr(), mb->length(), remote_addr_ );
    }
}

void
DeviceProxy::handle_lifecycle_dgram( ACE_Message_Block * mb )
{
    acewrapper::scoped_mblock_ptr<> will_release_ptr( mb );
    
	adportable::protocol::LifeCycleData data;
	adportable::protocol::LifeCycleFrame frame;
    
    TAO_InputCDR tao_input( mb );
    InputCDR input( tao_input );
	if ( lifecycle_frame_serializer::unpack( input, frame, data ) ) {
        
		// std::string o = adportable::protocol::LifeCycleHelper::to_string( data );
        
		adportable::protocol::LifeCycleCommand replyCmd;
		adportable::protocol::LifeCycleState newState;
		if ( lifeCycle_.dispatch_received_data( data, newState, replyCmd ) ) // state may change
			lifeCycle_.current_state( newState );
        
		if ( replyCmd != adportable::protocol::NOTHING ) {
			adportable::protocol::LifeCycleData reqData;
            if ( lifeCycle_.prepare_reply_data( replyCmd, reqData, 0 ) ) {
                lifeCycle_.apply_command( replyCmd, newState ); // state may change
                
                boost::intrusive_ptr<ACE_Message_Block> mb( lifecycle_frame_serializer::pack( reqData ) );
				dgram_handler_->send( mb->rd_ptr(), mb->length(), remote_addr_ );
            }
		}
        
		if ( adportable::protocol::LifeCycleHelper::command( data ) == adportable::protocol::DATA ) {
			unsigned long clsid;
            input >> clsid;
            handle_data( clsid, tao_input );
            if ( clsid == TOFConstants::ClassID_ProfileData ) {
                mb->rd_ptr( adportable::protocol::LifeCycle::wr_offset() );  // seek to the metadata
                pTask_->push_profile_data( mb->duplicate() );
            }
		}
        
	}
}

bool
DeviceProxy::handle_data( unsigned long clsid, TAO_InputCDR& cdr )
{
	if ( clsid == GlobalConstants::ClassID_InstEvent ) {

		TOFInstrument::InstEvent e;
		cdr >> e;
              
		std::wostringstream o;
        o << L"event#" << e.eventId_ << L", value=" << e.eventValue_;
        pTask_->dispatch_debug( o.str(), name() );

    } else if ( clsid == TOFConstants::ClassID_AnalyzerDeviceData ) {

        TOFInstrument::AnalyzerDeviceData d;
        cdr >> d;
        pTask_->setAnalyzerDeviceData( d );
        pTask_->device_update_notification( clsid );

		std::wostringstream o;
        o << L"AnalyzerDeviceData accel:" << d.accel_voltage;
        pTask_->dispatch_debug( o.str(), name() );

		/// SignalObserver debug
		pTask_->device_update_data();
    } else if ( clsid == TOFConstants::ClassID_ProfileData ) {
        size_t size = ( cdr.length() - adportable::protocol::LifeCycle::wr_offset() ) / 4;
		std::wostringstream o;
        o << L"TOF PROFILE DATA: len=" << size - 32;
        pTask_->dispatch_debug( o.str(), name() );

    } else {

		std::wostringstream o;
        o << L"Unknown clsid:" << std::hex << clsid;
        pTask_->dispatch_debug( o.str(), name() );

    }
	return true;
}

TAO_OutputCDR&
DeviceProxy::prepare_data( TAO_OutputCDR& out )
{
	adportable::protocol::LifeCycleData data;
	if ( lifeCycle_.prepare_data( data ) )
        acewrapper::lifecycle_frame_serializer::pack( out, data );
    return out;
}
