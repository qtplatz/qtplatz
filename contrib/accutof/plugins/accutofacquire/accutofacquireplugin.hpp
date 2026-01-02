
#pragma once

#include <extensionsystem/iplugin.h>
#include <memory>

namespace accutof {
    namespace acquire {

        class MainWindow;
        class iSequenceImpl;
        class Mode;

        class acquirePlugin : public ExtensionSystem::IPlugin
        {
            Q_OBJECT
            Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "accutofacquire.json")

            public:
            acquirePlugin();
            ~acquirePlugin();

            Utils::Result<> initialize(const QStringList &arguments) override;
            void extensionsInitialized() override;
            ShutdownFlag aboutToShutdown() override;
        private:
            std::unique_ptr< Mode > mode_;
            MainWindow * mainWindow_;

        private slots:
            void triggerAction();
        };

    } // namespace Internal
} // namespace u5303
