#ifndef CHEMSPIDER_HPP
#define CHEMSPIDER_HPP

#include "chemspider_global.hpp"

#include <extensionsystem/iplugin.h>

namespace ChemSpider {
namespace Internal {

class ChemSpiderPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    
public:
    ChemSpiderPlugin();
    ~ChemSpiderPlugin();
    
    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
	// ShutdownFlag aboutToShutdown();
    
private slots:
    void triggerAction();
};

} // namespace Internal
} // namespace ChemSpider

#endif // CHEMSPIDER_HPP

