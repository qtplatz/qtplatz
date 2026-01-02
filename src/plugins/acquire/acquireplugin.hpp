
#pragma once

#include "acquire_global.hpp"
#include <extensionsystem/iplugin.h>
#include <memory>

namespace acquire {

    class Mode;
    class MainWindow;
    class iSequenceImpl;

    class acquireplugin : public ExtensionSystem::IPlugin   {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "Acquire.json")

    public:
        acquireplugin();
        ~acquireplugin();

        Utils::Result<> initialize(const QStringList &arguments) override;
        void extensionsInitialized() override;
        ShutdownFlag aboutToShutdown() override;
    private:
        std::unique_ptr< Mode > mode_;
        MainWindow * mainWindow_;

    private slots:
        void triggerAction();
    };

} // namespace u5303
