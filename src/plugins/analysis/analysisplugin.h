// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef ANALYSISPLUGIN_H
#define ANALYSISPLUGIN_H

#include <extensionsystem/iplugin.h>
#include <boost/smart_ptr.hpp>

namespace Analysis {
    namespace internal {

      class AnalysisManager;

      class AnalysisPlugin : public ExtensionSystem::IPlugin {
	Q_OBJECT
      public:
        ~AnalysisPlugin();
        explicit AnalysisPlugin();
        void extensionsInitialized();

        bool initialize(const QStringList &arguments, QString *error_message);
      signals:

      public slots:

      private:
      boost::shared_ptr<AnalysisManager> manager_;

      };

    }
}

#endif // ANALYSISPLUGIN_H
