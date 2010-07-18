// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DEVICEPROXY_H
#define DEVICEPROXY_H

#include <ace/INET_Addr.h>
#include <adportable/protocollifecycle.h>

namespace acewrapper {
    template<class T> class EventHandler;
    template<class T> class DgramReceiver;
    class DgramHandler;
}

class QEventReceiver;

class DeviceProxy {
   public:
      DeviceProxy( const ACE_INET_Addr& );
      void update_device( const adportable::protocol::LifeCycleFrame&
			  , const adportable::protocol::LifeCycleData& );
      bool initialize();

   private:
      unsigned short remote_sequence_;
      unsigned short local_sequence_;

      boost::shared_ptr< acewrapper::EventHandler< acewrapper::DgramReceiver<QEventReceiver> > > dgramHandler_;
      ACE_INET_Addr remote_addr_;
      adportable::protocol::LifeCycle lifeCycle_;
};

#endif // DEVICEPROXY_H
