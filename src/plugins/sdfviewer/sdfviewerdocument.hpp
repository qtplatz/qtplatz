// Copyright (C) 2016 Denis Mingulov.
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <coreplugin/idocument.h>

namespace sdfviewer {

    class SDFViewerDocument : public Core::IDocument {
        Q_OBJECT

    public:

        SDFViewerDocument();
        ~SDFViewerDocument() override;
        Utils::Result<> open( const Utils::FilePath &filePath
                              , const Utils::FilePath &realFilePath) override;

        ReloadBehavior reloadBehavior(ChangeTrigger state, ChangeType type) const override;
        Utils::Result<> reload(ReloadFlag flag, ChangeType type) override;

        void updateVisibility();

    signals:
        void openFinished(bool success);

    private:
        void cleanUp();
    };

} // SDFViewer::Internal
