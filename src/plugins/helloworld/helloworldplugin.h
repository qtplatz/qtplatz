// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <extensionsystem/iplugin.h>

namespace HelloWorld {
namespace Internal {

class HelloMode;

class HelloWorldPlugin
  : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "HelloWorld.json")

public:
    HelloWorldPlugin();
    ~HelloWorldPlugin() override;

    bool initialize(const QStringList &arguments, QString *errorMessage) override;

    void extensionsInitialized() override;

private:
    void sayHelloWorld();

    HelloMode *m_helloMode = nullptr;
};

} // namespace Internal
} // namespace HelloWorld
