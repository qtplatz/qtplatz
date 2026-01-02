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
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "Peptide.json")

        public:
        peptideplugin();
        ~peptideplugin();

        Utils::Result<> initialize(const QStringList &arguments) override;
        void extensionsInitialized() override;
        ShutdownFlag aboutToShutdown() override;

    private slots:
        void triggerAction();
    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

} // namespace peptide

#endif // PEPTIDE_HPP
