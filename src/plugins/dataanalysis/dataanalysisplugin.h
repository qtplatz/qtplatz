#ifndef DATAANALYSISPLUGIN_H
#define DATAANALYSISPLUGIN_H

#include <extensionsystem/iplugin.h>

namespace DataAnalysis {
	namespace Internal {

                class DataAnalysisPlugin
                    : public ExtensionSystem::IPlugin {
                        Q_OBJECT
                public:
                        ~DataAnalysisPlugin();
			DataAnalysisPlugin();

			bool initialize(const QStringList &arguments, QString *error_message);
			void extensionsInitialized();
                private slots:
                        void sayHelloWorld();
                };
	}
}

#endif // DATAANALYSISPLUGIN_H
