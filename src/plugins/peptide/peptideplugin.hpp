#ifndef PEPTIDE_HPP
#define PEPTIDE_HPP

#include "peptide_global.hpp"

#include <extensionsystem/iplugin.h>
#include <memory>

namespace peptide {

    class MainWindow;
    class PeptideMode;

    class peptideplugin : public ExtensionSystem::IPlugin {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "peptide.json")
        
        public:
        peptideplugin();
        ~peptideplugin();
        
        bool initialize(const QStringList &arguments, QString *errorString);
        void extensionsInitialized();
        ShutdownFlag aboutToShutdown();
                                      
    private slots:
        void triggerAction();
    private:
        MainWindow * mainWindow_;
        std::shared_ptr< PeptideMode > mode_;
    };
    
} // namespace peptide

#endif // PEPTIDE_HPP

