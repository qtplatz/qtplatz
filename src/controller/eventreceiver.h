// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef EVENTRECEIVER_H
#define EVENTRECEIVER_H

#include <QObject>
#include <ace/Event_Handler.h>
#include <ace/INET_Addr.h>
#include <ace/Time_Value.h>

class ACE_Time_Value;

namespace acewrapper {
  class McastHandler;
  class DgramHandler;
}

class QEventReceiver : public QObject {
      Q_OBJECT
   public:
      explicit QEventReceiver(QObject *parent = 0);
      
      int handle_input(acewrapper::McastHandler&, ACE_HANDLE );  // routed from multicast handler
      int handle_input(acewrapper::DgramHandler&, ACE_HANDLE );  // routed from dgram handler
      int handle_input( ACE_HANDLE ); // native entry, may not be used
      int handle_timeout( const ACE_Time_Value&, const void * );
      int handle_close( ACE_HANDLE, ACE_Reactor_Mask );
      
  signals:
      void signal_dgram_input( ACE_Message_Block * mb );
      void signal_mcast_input( ACE_Message_Block * mb );
      void signal_timeout( unsigned long, long );

  public slots:
};

#endif // EVENTRECEIVER_H
