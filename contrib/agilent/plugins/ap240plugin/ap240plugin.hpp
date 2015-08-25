#ifndef U5303_HPP
#define U5303_HPP

#include "ap240_global.hpp"
#include <extensionsystem/iplugin.h>
#include <memory>

namespace adextension { class iSequenceImpl; }

namespace ap240 {

    class ap240Mode;
    class MainWindow;
    class iSequenceImpl;

    namespace Internal {

    class ap240Plugin : public ExtensionSystem::IPlugin
    {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "ap240plugin.json")

        public:
        ap240Plugin();
        ~ap240Plugin();

        bool initialize(const QStringList &arguments, QString *errorString);
        void extensionsInitialized();
        ShutdownFlag aboutToShutdown();
    private:
        std::shared_ptr< ap240Mode > mode_;
        std::unique_ptr< adextension::iSequenceImpl > iSequenceImpl_;
        MainWindow * mainWindow_;

    private slots:
        void triggerAction();
    };

} // namespace Internal
} // namespace u5303

#endif // U5303_HPP

