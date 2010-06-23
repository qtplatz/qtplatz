#ifndef ADBROKERPLUGIN_H
#define ADBROKERPLUGIN_H

#include "adbroker_global.h"
#include <extensionsystem/iplugin.h>

namespace ADBroker {

    namespace internal {

class ADBROKERSHARED_EXPORT ADBrokerPlugin : public ExtensionSystem::IPlugin {
    Q_OBJECT
public:
    ~ADBrokerPlugin();
    explicit ADBrokerPlugin();

    void extensionsInitialized();
    bool initialize(const QStringList &arguments, QString *error_message);
signals:

public slots:

};

}
}

#endif // ADBROKERPLUGIN_H
