// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef ANALYSISPLUGIN_H
#define ANALYSISPLUGIN_H

#include <extensionsystem/iplugin.h>

namespace Analysis {
    namespace internal {

      class AnalysisPlugin : public ExtensionSystem::IPlugin {
	Q_OBJECT
      public:
	~AnalysisPlugin();
        explicit AnalysisPlugin();

	bool initialize(const QStringList &arguments, QString *error_message);
      signals:

      public slots:

      };

    }
}

#endif // ANALYSISPLUGIN_H
