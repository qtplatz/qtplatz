#ifndef EXAMPLEPLUGIN_HPP
#define EXAMPLEPLUGIN_HPP

#include "example_global.hpp"

#include <extensionsystem/iplugin.h>

namespace Example {
namespace Internal {

class ExamplePlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "Example.json")

public:
    ExamplePlugin();
    ~ExamplePlugin() override;

    bool initialize(const QStringList &arguments, QString *errorString) override;
    void extensionsInitialized() override;
    ShutdownFlag aboutToShutdown() override;

private:
    void triggerAction();
};

} // namespace Internal
} // namespace Example

#endif // EXAMPLEPLUGIN_HPP
