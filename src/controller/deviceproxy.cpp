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

    if ( lifeCycle_.current_state() == LCS_CLOSED || lifeCycle_.current_state() == LCS_SYN_SENT ) {

        if ( ! dgramHandler_ )
            initialize();

        LifeCycleState next;
        if ( lifeCycle_.apply_command( CONN_SYN, next ) ) { // state has moved
            // first trial only through this section
        }
        LifeCycle_SYN reqData;
        ACE_Message_Block * mb = lifecycle_frame_serializer::pack( LifeCycleData(reqData) );
        dgramHandler_->send( mb->rd_ptr(), mb->length(), remote_addr_ );
        ACE_Message_Block::release( mb );
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
   }
   return true;
 }
