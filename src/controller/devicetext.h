// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DEVICETEXT_H
#define DEVICETEXT_H

#include <QPlainTextEdit>

class DeviceText : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit DeviceText(QWidget *parent = 0);

signals:

public slots:
    void handle_debug( std::string remote_addr, QString description );
};

#endif // DEVICETEXT_H
