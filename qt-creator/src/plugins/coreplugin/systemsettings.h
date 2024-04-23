// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "core_global.h"

#include <utils/aspects.h>

namespace Core::Internal {

class CORE_TEST_EXPORT SystemSettings final : public Utils::AspectContainer
{
public:
    SystemSettings();

    Utils::FilePathAspect patchCommand{this};

    Utils::BoolAspect autoSaveModifiedFiles{this};
    Utils::IntegerAspect autoSaveInterval{this};

    Utils::BoolAspect autoSaveAfterRefactoring{this};

    Utils::BoolAspect autoSuspendEnabled{this};
    Utils::IntegerAspect autoSuspendMinDocumentCount{this};

    Utils::BoolAspect warnBeforeOpeningBigFiles{this};
    Utils::IntegerAspect bigFileSizeLimitInMB{this};

    Utils::IntegerAspect maxRecentFiles{this};

    Utils::SelectionAspect reloadSetting{this};

#ifdef ENABLE_CRASHPAD
    Utils::BoolAspect enableCrashReporting{this};
    Utils::BoolAspect showCrashButton{this};
#endif

    Utils::BoolAspect askBeforeExit{this};
};

CORE_TEST_EXPORT SystemSettings &systemSettings();

} // Core::Internal
