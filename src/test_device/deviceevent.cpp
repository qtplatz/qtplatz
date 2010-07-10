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
    //emit signal_2(pbuf, octets, &addr);
    //emit signal_3(pbuf, octets, &addr);
}
