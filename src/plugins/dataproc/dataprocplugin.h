// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#pragma once

#include <extensionsystem/iplugin.h>
#include <boost/smart_ptr.hpp>

namespace adportable {
	class Configuration;
}

namespace dataproc {
    namespace internal {

      class DataprocManager;

      class DataprocPlugin : public ExtensionSystem::IPlugin {

	Q_OBJECT
      public:
        ~DataprocPlugin();
        explicit DataprocPlugin();
        void extensionsInitialized();

        bool initialize(const QStringList &arguments, QString *error_message);
      signals:

      public slots:

      private:
      boost::shared_ptr<DataprocManager> manager_;
      boost::shared_ptr< adportable::Configuration > pConfig_;

      };

    }
}

