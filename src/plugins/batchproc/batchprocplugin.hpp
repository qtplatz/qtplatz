#ifndef BATCHPROC_HPP
#define BATCHPROC_HPP

#include "batchproc_global.hpp"

#include <extensionsystem/iplugin.h>

namespace batchproc {

    class batchprocPlugin : public ExtensionSystem::IPlugin {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "batchproc.json")
        
        public:
        batchprocPlugin();
        ~batchprocPlugin();
        
        bool initialize(const QStringList &arguments, QString *errorString);
        void extensionsInitialized();
        ShutdownFlag aboutToShutdown();
                                      
    private slots:
        void triggerAction();
    };
    
} // namespace batchproc

#endif // BATCHPROC_HPP

