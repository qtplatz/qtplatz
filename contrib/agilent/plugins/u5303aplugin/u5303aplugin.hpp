#ifndef U5303_HPP
#define U5303_HPP

#include "u5303a_global.hpp"
#include <extensionsystem/iplugin.h>
#include <memory>


namespace u5303a {

    class u5303AMode;
    class MainWindow;
    class iSequenceImpl;

    namespace Internal {

    class u5303APlugin : public ExtensionSystem::IPlugin
    {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "u5303aplugin.json")

        public:
        u5303APlugin();
        ~u5303APlugin();

        bool initialize(const QStringList &arguments, QString *errorString);
        void extensionsInitialized();
        ShutdownFlag aboutToShutdown();
    private:
        std::shared_ptr< u5303AMode > mode_;
        MainWindow * mainWindow_;

    private slots:
        void triggerAction();
    };

} // namespace Internal
} // namespace u5303

#endif // U5303_HPP

