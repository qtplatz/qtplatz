//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "deviceevent.h"
#include <acewrapper/callback.h>

DeviceEvent::DeviceEvent(QObject *parent) : QObject(parent)
{
}

//virtual
void
DeviceEvent::operator()(const char * pbuf, ssize_t octets, const ACE_INET_Addr& addr)
{
    emit signal_mcast(pbuf, octets, &addr);
}

int
DeviceEvent::handle_timeout( const ACE_Time_Value& tv, const void * arg )
{
   Q_UNUSED(arg);
   emit signal_timeout( &tv );
   return 0;
}

int
DeviceEvent::handle_input(ACE_HANDLE)
{ 
   return 0;
}

int
DeviceEvent::handle_close( ACE_HANDLE handle, ACE_Reactor_Mask close_mask )
{
   Q_UNUSED(handle);
   Q_UNUSED(close_mask); 
   return 0;
}
