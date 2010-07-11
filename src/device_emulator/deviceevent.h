// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DEVICEEVENT_H
#define DEVICEEVENT_H

#include <QObject>
#include <acewrapper/callback.h>
#include <ace/Time_Value.h>
#include <ace/Reactor.h>

class DeviceEvent : public QObject, public acewrapper::Callback {
    Q_OBJECT
public:
    explicit DeviceEvent(QObject *parent = 0);
    virtual void operator()(const char * pbuf, ssize_t, const ACE_INET_Addr& );

      // for EventHandler :: ACE_Event_Handler
      ACE_HANDLE get_handle() const { return 0; }
      int handle_input(ACE_HANDLE);
      int handle_timeout( const ACE_Time_Value& tv, const void * arg );
      int handle_close( ACE_HANDLE handle, ACE_Reactor_Mask mask);

signals:
      void signal_mcast( const char *, int, const ACE_INET_Addr* );
      void signal_timeout( const ACE_Time_Value * );

public:
      unsigned long timerId_;
};

#endif // DEVICEEVENT_H
