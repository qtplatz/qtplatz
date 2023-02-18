// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include <QWidget>

#include <functional>

namespace Utils {

class MacroExpander;

namespace Internal { class VariableChooserPrivate; }

class QTCREATOR_UTILS_EXPORT VariableChooser : public QWidget
{
    Q_OBJECT

public:
    explicit VariableChooser(QWidget *parent = nullptr);
    ~VariableChooser() override;

    void addMacroExpanderProvider(const std::function<MacroExpander *()> &provider);
    void addSupportedWidget(QWidget *textcontrol, const QByteArray &ownName = QByteArray());

    static void addSupportForChildWidgets(QWidget *parent, MacroExpander *expander);

protected:
    bool event(QEvent *ev) override;
    bool eventFilter(QObject *, QEvent *event) override;

private:
    Internal::VariableChooserPrivate *d;
};

} // namespace Utils
