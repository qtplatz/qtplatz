// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <extensionsystem/iplugin.h>

namespace figshare {

    class Mode;

    class FigsharePlugin
        : public ExtensionSystem::IPlugin
    {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "figshare.json")

        public:
        FigsharePlugin();
        ~FigsharePlugin() override;

        bool initialize(const QStringList &arguments, QString *errorMessage) override;

        void extensionsInitialized() override;

    private:
        void sayHello();
        class impl;
        impl * impl_;
    };

} // namespace rest
