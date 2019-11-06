
#pragma once
#include "aqmd3_global.hpp"
#include <extensionsystem/iplugin.h>
#include <memory>


namespace aqmd3 {

    class aqmd3Mode;
    class MainWindow;
    class iSequenceImpl;

    class aqmd3Plugin : public ExtensionSystem::IPlugin   {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "aqmd3plugin.json")

        public:
        aqmd3Plugin();
        ~aqmd3Plugin();

        bool initialize(const QStringList &arguments, QString *errorString);
        void extensionsInitialized();
        ShutdownFlag aboutToShutdown();
    private:
        std::shared_ptr< aqmd3Mode > mode_;
        MainWindow * mainWindow_;

    private slots:
        void triggerAction();
    };

}
