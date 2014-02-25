#ifndef U5303_HPP
#define U5303_HPP

#include "u5303a_global.hpp"

#include <extensionsystem/iplugin.h>

namespace u5303a {
namespace Internal {

class u5303APlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "u5303a.json")

public:
    u5303APlugin();
    ~u5303APlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();

private slots:
    void triggerAction();
};

} // namespace Internal
} // namespace u5303

#endif // U5303_HPP

