// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include <QWidget>

namespace Utils {

class QTCREATOR_UTILS_EXPORT FakeToolTip : public QWidget
{
    Q_OBJECT

public:
    explicit FakeToolTip(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
};

} // namespace Utils
