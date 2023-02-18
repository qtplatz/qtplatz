// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "processenums.h"

#include <QIODevice>
#include <QProcess>

namespace Utils {

class ProcessStartHandler
{
public:
    ProcessStartHandler(QProcess *process) : m_process(process) {}

    void setProcessMode(ProcessMode mode) { m_processMode = mode; }
    void setWriteData(const QByteArray &writeData) { m_writeData = writeData; }
    QIODevice::OpenMode openMode() const;
    void handleProcessStart();
    void handleProcessStarted();
    void setBelowNormalPriority();
    void setNativeArguments(const QString &arguments);

private:
    ProcessMode m_processMode = ProcessMode::Reader;
    QByteArray m_writeData;
    QProcess *m_process;
};

class ProcessHelper : public QProcess
{
    Q_OBJECT

public:
    explicit ProcessHelper(QObject *parent);

    ProcessStartHandler *processStartHandler() { return &m_processStartHandler; }

    using QProcess::setErrorString;

    void setLowPriority() { m_lowPriority = true; }
    void setUnixTerminalDisabled() { m_unixTerminalDisabled = true; }
    void setUseCtrlCStub(bool enabled); // release only

    static void terminateProcess(QProcess *process);
    static void interruptProcess(QProcess *process);
    static void interruptPid(qint64 pid);

private:
    void terminateProcess();
    void setupChildProcess_impl();

    bool m_lowPriority = false;
    bool m_unixTerminalDisabled = false;
    bool m_useCtrlCStub = false;
    ProcessStartHandler m_processStartHandler;
};

} // namespace Utils
