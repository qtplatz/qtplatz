// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DEVICEEVENT_H
#define DEVICEEVENT_H

#include <QObject>

class DeviceEvent : public QObject {
    Q_OBJECT
public:
    explicit DeviceEvent(QObject *parent = 0);

signals:
    void signal_notify();

public slots:
    void slotRaiseEvent();

};

#endif // DEVICEEVENT_H
