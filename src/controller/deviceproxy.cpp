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
{
}

void
DeviceProxy::update_device( const LifeCycleFrame& frame, const LifeCycleData& data )
{
    ACE_UNUSED_ARG(data);
    ACE_UNUSED_ARG(frame);
    using namespace acewrapper;

    if ( lifeCycle_.current_state() == LCS_CLOSED || lifeCycle_.current_state() == LCS_SYN_SENT ) {

        if ( ! dgramHandler_ )
            initialize();

        LifeCycleState next;
        if ( lifeCycle_.apply_command( CONN_SYN, next ) ) { // state has moved
            // first trial only through this section
        }
        LifeCycle_SYN reqData;
        reqData.remote_sequence_ = 0;
        reqData.sequence_ = 0x100;
        boost::intrusive_ptr<ACE_Message_Block> mb( lifecycle_frame_serializer::pack( LifeCycleData(reqData) ) );
#if defined _DEBUG
        do {
           LifeCycleFrame dbgFrame;
           LifeCycleData dbgData = LifeCycle_SYN();
           lifecycle_frame_serializer::unpack( mb.get(), dbgFrame, dbgData );
           LifeCycle_SYN& syn = boost::get<LifeCycle_SYN&>( dbgData );
           long x = 0;
        } while(0);
#endif

        dgramHandler_->send( mb->rd_ptr(), mb->length(), remote_addr_ );

        emit signal_dgram_to_device( remote_addr_string_, QString(local_addr_string_.c_str()), "SYN sent" );

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

    do { // debug diagnostic
        ACE_Message_Block * pfrom = mb->cont();
        ACE_INET_Addr& from_addr( *reinterpret_cast<ACE_INET_Addr *>( pfrom->rd_ptr() ) );
        std::string remote_addr = acewrapper::string( from_addr );
        assert( remote_addr == remote_addr_string_ );
    } while (0);

    LifeCycleData data;
    LifeCycleFrame frame;
   
    lifecycle_frame_serializer::unpack( mb, frame, data );
    std::string o = LifeCycleHelper::to_string( data );
    emit signal_dgram_to_device( remote_addr_string_, QString(local_addr_string_.c_str()), o.c_str() );
}
