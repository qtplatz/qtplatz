//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "deviceevent.h"

DeviceEvent::DeviceEvent(QObject *parent) :
    QObject(parent)
{
}

void
DeviceEvent::slotRaiseEvent()
{
   emit signal_notify();
}

