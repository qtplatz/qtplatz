// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <coreplugin/core_global.h>

#include <utils/id.h>

#include <QFuture>
#include <QFutureInterfaceBase>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(QTimer)

namespace Core {
class FutureProgress;

namespace Internal { class ProgressManagerPrivate; }

class CORE_EXPORT ProgressManager : public QObject
{
    Q_OBJECT
public:
    enum ProgressFlag {
        KeepOnFinish = 0x01,
        ShowInApplicationIcon = 0x02
    };
    Q_DECLARE_FLAGS(ProgressFlags, ProgressFlag)

    static ProgressManager *instance();

    template <typename T>
    static FutureProgress *addTask(const QFuture<T> &future, const QString &title,
                                   Utils::Id type, ProgressFlags flags = {}) {
        return addTask(QFuture<void>(future), title, type, flags);
    }

    static FutureProgress *addTask(const QFuture<void> &future, const QString &title,
                                   Utils::Id type, ProgressFlags flags = {});
    static FutureProgress *addTimedTask(const QFutureInterface<void> &fi, const QString &title,
                                        Utils::Id type, int expectedSeconds, ProgressFlags flags = {});
    static void setApplicationLabel(const QString &text);

public slots:
    static void cancelTasks(Utils::Id type);

signals:
    void taskStarted(Utils::Id type);
    void allTasksFinished(Utils::Id type);

private:
    ProgressManager();
    ~ProgressManager() override;

    friend class Core::Internal::ProgressManagerPrivate;
};

class CORE_EXPORT ProgressTimer : public QObject
{
public:
    ProgressTimer(const QFutureInterfaceBase &futureInterface, int expectedSeconds,
                  QObject *parent = nullptr);

private:
    void handleTimeout();

    QFutureInterfaceBase m_futureInterface;
    int m_expectedTime;
    int m_currentTime = 0;
    QTimer *m_timer;
};

} // namespace Core

Q_DECLARE_OPERATORS_FOR_FLAGS(Core::ProgressManager::ProgressFlags)
