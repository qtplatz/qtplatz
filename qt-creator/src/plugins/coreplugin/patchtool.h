// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "core_global.h"

#include <utils/filepath.h>

namespace Core {

enum class PatchAction {
    Apply,
    Revert
};

class CORE_EXPORT PatchTool
{
public:
    static Utils::FilePath patchCommand();
    static void setPatchCommand(const Utils::FilePath &newCommand);

    static bool confirmPatching(QWidget *parent, PatchAction patchAction);

    // Utility to run the 'patch' command
    static bool runPatch(const QByteArray &input, const Utils::FilePath &workingDirectory = {},
                         int strip = 0, PatchAction patchAction = PatchAction::Apply);
};

} // namespace Core
