#ifndef FREQUENCY_HPP
#define FREQUENCY_HPP

#include "frequency_global.hpp"
#include <extensionsystem/iplugin.h>
#include <boost/smart_ptr.hpp>

namespace frequency {

    class Mode;
    class MainWindow;

    class frequencyPlugin : public ExtensionSystem::IPlugin {
        Q_OBJECT
    
    public:
        frequencyPlugin();
        ~frequencyPlugin();
        
        bool initialize(const QStringList &arguments, QString *errorString);
        void extensionsInitialized();
                                    
    private slots:
        void triggerAction();
    private:
        boost::scoped_ptr< Mode > mode_;
        boost::scoped_ptr< MainWindow > mainWindow_;
    };

} // namespace frequency

#endif // FREQUENCY_HPP

