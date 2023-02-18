// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <coreplugin/dialogs/ioptionspage.h>

namespace Core {
namespace Internal {

class GeneralSettings : public IOptionsPage
{
public:
    GeneralSettings();

    static bool showShortcutsInContextMenu();
    void setShowShortcutsInContextMenu(bool show);

private:
    friend class GeneralSettingsWidget;
    bool m_defaultShowShortcutsInContextMenu;
};

} // namespace Internal
} // namespace Core
