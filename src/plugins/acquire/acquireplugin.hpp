
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "Acquire.json")
#else
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "acquireplugin.json")
#endif
        public:
        acquireplugin();
        ~acquireplugin();

        bool initialize(const QStringList &arguments, QString *errorString);
        void extensionsInitialized();
        ShutdownFlag aboutToShutdown();
    private:
        std::unique_ptr< Mode > mode_;
        MainWindow * mainWindow_;

    private slots:
        void triggerAction();
    };

} // namespace u5303
