// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DEVICEPROXY_H
#define DEVICEPROXY_H

#include <ace/INET_Addr.h>
#include <adportable/protocollifecycle.h>
#include <QObject>
class ACE_Message_Block;

namespace acewrapper {
    template<class T> class EventHandler;
    template<class T> class DgramReceiver;
    class DgramHandler;
}

class QEventReceiver;

class DeviceProxy : public QObject {
      Q_OBJECT
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
      std::string remote_addr_string_;
      std::string local_addr_string_;

   private slots:
      void on_notify_dgram( ACE_Message_Block * mb );

   signals:
      void signal_dgram_to_device( std::string remote_addr, QString local_address, QString description );
};

#endif // DEVICEPROXY_H
