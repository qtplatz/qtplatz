// This is a -*- C++ -*- header.

#ifndef DATAANALYSISPLUGIN_H
#define DATAANALYSISPLUGIN_H

#include <extensionsystem/iplugin.h>
#include <coreplugin/basemode.h>

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

		/////////////////////////////

		class DataAnalysisMode : public Core::BaseMode {
			Q_OBJECT
		public:
			~DataAnalysisMode();
			DataAnalysisMode( QObject * parent = 0 );
		};

	}
}

#endif // DATAANALYSISPLUGIN_H
