// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "core_global.h"
#include "ioutputpane.h"

#include <QMetaType>
#include <QObject>

QT_BEGIN_NAMESPACE
class QFont;
QT_END_NAMESPACE

namespace Core {

namespace Internal { class MainWindow; }

class CORE_EXPORT MessageManager : public QObject
{
    Q_OBJECT

public:
    static MessageManager *instance();

    static void setFont(const QFont &font);
    static void setWheelZoomEnabled(bool enabled);

    static void writeSilently(const QString &message);
    static void writeFlashing(const QString &message);
    static void writeDisrupting(const QString &message);

    static void writeSilently(const QStringList &messages);
    static void writeFlashing(const QStringList &messages);
    static void writeDisrupting(const QStringList &messages);

private:
    MessageManager();
    ~MessageManager() override;

    static void init();
    friend class Core::Internal::MainWindow;
};

} // namespace Core
