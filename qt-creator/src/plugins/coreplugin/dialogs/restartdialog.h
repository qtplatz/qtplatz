// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <coreplugin/core_global.h>

#include <QCoreApplication>
#include <QMessageBox>

namespace Core {

class CORE_EXPORT RestartDialog : public QMessageBox
{
    Q_DECLARE_TR_FUNCTIONS(Core::RestartDialog)

public:
    RestartDialog(QWidget *parent, const QString &text);
};

} // namespace Core
