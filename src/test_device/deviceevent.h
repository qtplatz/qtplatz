// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DEVICEEVENT_H
#define DEVICEEVENT_H

#include <QObject>
#include <acewrapper/callback.h>

class DeviceEvent : public QObject, public acewrapper::Callback {
    Q_OBJECT
public:
    explicit DeviceEvent(QObject *parent = 0);
    virtual void operator()(const char * pbuf, ssize_t, const ACE_INET_Addr& );

signals:
    void signal_mcast( const char *, int, const ACE_INET_Addr* );
};

#endif // DEVICEEVENT_H
